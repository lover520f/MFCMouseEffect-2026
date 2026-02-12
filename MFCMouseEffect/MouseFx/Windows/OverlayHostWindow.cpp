#include "pch.h"

#include "OverlayHostWindow.h"
#include "MouseFx/Core/OverlayCoordSpace.h"
#include "MouseFx/Gpu/DawnCommandConsumer.h"
#include "MouseFx/Gpu/GpuFinalPresentCapabilityProbe.h"
#include "MouseFx/Gpu/GpuFinalPresentHostChain.h"
#include "MouseFx/Gpu/GpuFinalPresentOptIn.h"
#include "MouseFx/Gpu/GpuFinalPresentPolicy.h"
#include "MouseFx/Gpu/GpuFinalPresentTakeoverGate.h"

#include <algorithm>
#include <mmsystem.h>
#include <vector>

#pragma comment(lib, "winmm.lib")

namespace mousefx {
namespace {

struct ScreenRect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct MonitorEnumState {
    std::vector<ScreenRect> rects{};
};

static uint64_t NowMs() {
    return GetTickCount64();
}

BOOL CALLBACK EnumMonitorsProc(HMONITOR monitor, HDC, LPRECT, LPARAM data) {
    auto* state = reinterpret_cast<MonitorEnumState*>(data);
    if (!state) return TRUE;

    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(monitor, &mi)) return TRUE;

    ScreenRect rect{};
    rect.x = mi.rcMonitor.left;
    rect.y = mi.rcMonitor.top;
    rect.w = mi.rcMonitor.right - mi.rcMonitor.left;
    rect.h = mi.rcMonitor.bottom - mi.rcMonitor.top;
    if (rect.w > 0 && rect.h > 0) {
        state->rects.push_back(rect);
    }
    return TRUE;
}

std::vector<ScreenRect> QueryMonitorRects() {
    MonitorEnumState state{};
    EnumDisplayMonitors(nullptr, nullptr, &EnumMonitorsProc, reinterpret_cast<LPARAM>(&state));
    std::sort(state.rects.begin(), state.rects.end(), [](const ScreenRect& a, const ScreenRect& b) {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        if (a.w != b.w) return a.w < b.w;
        return a.h < b.h;
    });
    return state.rects;
}

void UnionRects(const std::vector<OverlayHostWindow::HostSurface>& surfaces, int* x, int* y, int* w, int* h) {
    if (!x || !y || !w || !h) return;
    if (surfaces.empty()) {
        *x = GetSystemMetrics(SM_XVIRTUALSCREEN);
        *y = GetSystemMetrics(SM_YVIRTUALSCREEN);
        *w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        *h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        return;
    }

    int left = surfaces[0].x;
    int top = surfaces[0].y;
    int right = surfaces[0].x + surfaces[0].width;
    int bottom = surfaces[0].y + surfaces[0].height;
    for (size_t i = 1; i < surfaces.size(); ++i) {
        const auto& s = surfaces[i];
        left = (std::min)(left, s.x);
        top = (std::min)(top, s.y);
        right = (std::max)(right, s.x + s.width);
        bottom = (std::max)(bottom, s.y + s.height);
    }
    *x = left;
    *y = top;
    *w = right - left;
    *h = bottom - top;
}

bool EnsureSurfaceBuffer(OverlayHostWindow::HostSurface& surface, int w, int h) {
    if (w <= 0 || h <= 0) return false;
    if (surface.memDc && surface.bits && surface.width == w && surface.height == h) return true;

    if (surface.dib) {
        DeleteObject(surface.dib);
        surface.dib = nullptr;
    }
    if (surface.memDc) {
        DeleteDC(surface.memDc);
        surface.memDc = nullptr;
    }
    surface.bits = nullptr;
    surface.width = 0;
    surface.height = 0;

    HDC screen = GetDC(nullptr);
    surface.memDc = CreateCompatibleDC(screen);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    surface.dib = CreateDIBSection(screen, &bmi, DIB_RGB_COLORS, &surface.bits, nullptr, 0);
    if (surface.dib) {
        SelectObject(surface.memDc, surface.dib);
    }
    ReleaseDC(nullptr, screen);

    if (!surface.memDc || !surface.dib || !surface.bits) {
        if (surface.dib) DeleteObject(surface.dib);
        if (surface.memDc) DeleteDC(surface.memDc);
        surface.dib = nullptr;
        surface.memDc = nullptr;
        surface.bits = nullptr;
        surface.width = 0;
        surface.height = 0;
        return false;
    }

    surface.width = w;
    surface.height = h;
    return true;
}

void RefreshSurfaceRect(OverlayHostWindow::HostSurface& surface) {
    if (!surface.hwnd) return;

    RECT rcWindow{};
    if (GetWindowRect(surface.hwnd, &rcWindow)) {
        surface.x = rcWindow.left;
        surface.y = rcWindow.top;
    }

    RECT rcClient{};
    if (GetClientRect(surface.hwnd, &rcClient)) {
        const int cw = rcClient.right - rcClient.left;
        const int ch = rcClient.bottom - rcClient.top;
        EnsureSurfaceBuffer(surface, cw, ch);
    }
}

static const uint64_t kTopmostReassertIntervalMs = 2500;
static constexpr UINT kFrameTimerIntervalDefaultMs = 8;
static constexpr UINT kFrameTimerIntervalHoldMs = 4;
static constexpr UINT kFrameTimerIntervalDawnActiveMs = 4;
static constexpr uint64_t kImmediateFrameKickMinIntervalMs = 2;
static constexpr uint64_t kGpuPresentRollbackCooldownMs = 2500;
static constexpr uint32_t kGpuPresentRollbackFailureThreshold = 2;
static OverlayHostWindow* g_overlayForegroundHookOwner = nullptr;
static const uint64_t g_processStartTickMs = GetTickCount64();

} // namespace

OverlayHostWindow::OverlayHostWindow() = default;

OverlayHostWindow::~OverlayHostWindow() {
    Shutdown();
}

uint64_t OverlayHostWindow::GetLastGpuCommandFrameTickMs() const {
    return gpuCommandFrameTickMs_.load(std::memory_order_acquire);
}

uint32_t OverlayHostWindow::GetLastGpuCommandCount() const {
    return gpuCommandCount_.load(std::memory_order_acquire);
}

uint32_t OverlayHostWindow::GetLastGpuTrailCommandCount() const {
    return gpuTrailCommandCount_.load(std::memory_order_acquire);
}

uint32_t OverlayHostWindow::GetLastGpuRippleCommandCount() const {
    return gpuRippleCommandCount_.load(std::memory_order_acquire);
}

uint32_t OverlayHostWindow::GetLastGpuParticleCommandCount() const {
    return gpuParticleCommandCount_.load(std::memory_order_acquire);
}

uint64_t OverlayHostWindow::GetGpuPresentAttemptCount() const {
    return gpuPresentAttemptCount_.load(std::memory_order_acquire);
}

uint64_t OverlayHostWindow::GetGpuPresentSuccessCount() const {
    return gpuPresentSuccessCount_.load(std::memory_order_acquire);
}

uint64_t OverlayHostWindow::GetGpuPresentFallbackCount() const {
    return gpuPresentFallbackCount_.load(std::memory_order_acquire);
}

std::string OverlayHostWindow::GetGpuPresentLastDetail() const {
    std::lock_guard<std::mutex> lock(gpuPresentDetailMutex_);
    return gpuPresentLastDetail_;
}

bool OverlayHostWindow::IsGpuPresentActive() const {
    return gpuPresentSuccessCount_.load(std::memory_order_acquire) > 0;
}

gpu::GpuFinalPresentPolicyDecision OverlayHostWindow::GetGpuFinalPresentPolicyDecision() const {
    return EvaluateGpuFinalPresentPolicy();
}

void OverlayHostWindow::RequestImmediateFrame() {
    if (!ticking_ || !timerHwnd_ || !IsWindow(timerHwnd_)) return;
    const uint64_t nowMs = NowMs();
    const uint64_t lastKickMs = lastImmediateFrameKickMs_.load(std::memory_order_acquire);
    if (lastKickMs != 0 && nowMs >= lastKickMs &&
        (nowMs - lastKickMs) < kImmediateFrameKickMinIntervalMs) {
        return;
    }
    if (immediateFrameKickPending_.exchange(true, std::memory_order_acq_rel)) {
        return;
    }
    lastImmediateFrameKickMs_.store(nowMs, std::memory_order_release);
    if (!PostMessageW(timerHwnd_, kMsgRequestImmediateFrame, 0, 0)) {
        immediateFrameKickPending_.store(false, std::memory_order_release);
    }
}

const wchar_t* OverlayHostWindow::ClassName() {
    return L"MouseFxOverlayHostWindow";
}

bool OverlayHostWindow::EnsureClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &OverlayHostWindow::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = ClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

bool OverlayHostWindow::Create() {
    if (timerHwnd_) return true;
    if (!EnsureClassRegistered()) return false;
    if (!RebuildSurfaces()) return false;

    for (auto& surface : surfaces_) {
        if (surface.hwnd) ShowWindow(surface.hwnd, SW_HIDE);
    }
    RegisterForegroundHook();
    SyncBoundsWithVirtualScreen(true);
    EnsureTopmostZOrder(true);
    return true;
}

void OverlayHostWindow::Shutdown() {
    StopFrameLoop();
    UnregisterForegroundHook();
    DestroySurfaces();
    layers_.clear();
    ClearOverlayWindowHandle();
    ClearOverlayOriginOverride();
}

IOverlayLayer* OverlayHostWindow::AddLayer(std::unique_ptr<IOverlayLayer> layer) {
    if (!layer) return nullptr;
    IOverlayLayer* raw = layer.get();
    layers_.push_back(std::move(layer));
    StartFrameLoop();
    return raw;
}

void OverlayHostWindow::RemoveLayer(IOverlayLayer* layer) {
    if (!layer) return;
    layers_.erase(
        std::remove_if(
            layers_.begin(),
            layers_.end(),
            [layer](const std::unique_ptr<IOverlayLayer>& item) { return item.get() == layer; }),
        layers_.end());
    if (layers_.empty()) {
        StopFrameLoop();
    }
}

void OverlayHostWindow::ClearLayers() {
    layers_.clear();
    StopFrameLoop();
}

void OverlayHostWindow::SetGpuSubmitContext(const std::string& activeBackend, const std::string& pipelineMode) {
    const bool backendChanged = (gpuSubmitActiveBackend_ != activeBackend);
    const bool pipelineChanged = (gpuSubmitPipelineMode_ != pipelineMode);
    gpuSubmitActiveBackend_ = activeBackend.empty() ? "cpu" : activeBackend;
    gpuSubmitPipelineMode_ = pipelineMode.empty() ? "cpu_layered" : pipelineMode;
    if (backendChanged || pipelineChanged) {
        pendingLayeredRollback_.store(false, std::memory_order_release);
        layeredCpuFallbackUntilMs_.store(0, std::memory_order_release);
        gpuPresentConsecutiveFailures_.store(0, std::memory_order_release);
        lastNonEmptyGpuCommandTickMs_.store(0, std::memory_order_release);
    }
    (void)EnsureSurfaceMode(false);
}

LRESULT CALLBACK OverlayHostWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OverlayHostWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<OverlayHostWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<OverlayHostWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) return self->OnMessage(hwnd, msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT OverlayHostWindow::OnMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_TIMER:
        if (wParam == kTimerId && hwnd == timerHwnd_) {
            OnTick();
            return 0;
        }
        break;
    case kMsgEnsureTopmost:
        EnsureTopmostZOrder(true);
        return 0;
    case kMsgRequestImmediateFrame:
        immediateFrameKickPending_.store(false, std::memory_order_release);
        if (ticking_ && hwnd == timerHwnd_) {
            OnTick();
        }
        return 0;
    case WM_DESTROY:
        if (hwnd == timerHwnd_) {
            ClearOverlayWindowHandle();
            timerHwnd_ = nullptr;
        }
        break;
    case WM_DISPLAYCHANGE:
    case WM_DPICHANGED:
        if (hwnd == timerHwnd_) {
            SyncBoundsWithVirtualScreen(true);
            return 0;
        }
        break;
    default:
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void OverlayHostWindow::OnTick() {
    if (layers_.empty()) {
        StopFrameLoop();
        return;
    }

    if (!EnsureSurfaceMode(false)) {
        return;
    }
    SyncBoundsWithVirtualScreen(false);
    EnsureTopmostZOrder(false);
    const uint64_t nowMs = NowMs();
    for (auto& layer : layers_) {
        layer->Update(nowMs);
    }
    layers_.erase(
        std::remove_if(
            layers_.begin(),
            layers_.end(),
            [](const std::unique_ptr<IOverlayLayer>& layer) { return !layer || !layer->IsAlive(); }),
        layers_.end());

    CollectGpuCommandStream(nowMs);
    UpdateFrameLoopTimerInterval(ResolveNextTimerIntervalMs());
    Render();
    if (pendingLayeredRollback_.exchange(false, std::memory_order_acq_rel)) {
        (void)EnsureSurfaceMode(true);
    }
}

void OverlayHostWindow::Render() {
    for (auto& surface : surfaces_) {
        RenderSurface(surface);
    }
}

void OverlayHostWindow::RenderSurface(HostSurface& surface) {
    if (!surface.hwnd || !surface.memDc || !surface.bits || surface.width <= 0 || surface.height <= 0) return;

    SetOverlayOriginOverride(surface.x, surface.y);

    std::vector<IOverlayLayer*> layerRefs{};
    layerRefs.reserve(layers_.size());
    for (const auto& layer : layers_) {
        if (layer) {
            layerRefs.push_back(layer.get());
        }
    }
    OverlayPresentFrame frame{};
    frame.hwnd = surface.hwnd;
    frame.memDc = surface.memDc;
    frame.bits = surface.bits;
    frame.width = surface.width;
    frame.height = surface.height;
    frame.surfaceX = surface.x;
    frame.surfaceY = surface.y;
    frame.hadVisibleContent = &surface.hadVisibleContent;
    frame.hadPresentedFrame = &surface.hadPresentedFrame;
    frame.layers = &layerRefs;
    frame.gpuCommandStream = &gpuCommandStream_;

    const bool dawnBackendSelected =
        (gpuSubmitActiveBackend_ == "dawn") &&
        (gpuSubmitPipelineMode_ == "dawn_compositor");
    const bool wantsGpuPresent = dawnBackendSelected && !useLayeredSurfaces_;
    bool gpuExclusiveEligible = true;
    bool hasVisibleLayerOnSurface = false;
    if (wantsGpuPresent) {
        const int left = surface.x;
        const int top = surface.y;
        const int right = surface.x + surface.width;
        const int bottom = surface.y + surface.height;
        for (IOverlayLayer* layer : layerRefs) {
            if (!layer || !layer->IsAlive()) continue;
            if (!layer->IntersectsScreenRect(left, top, right, bottom)) continue;
            hasVisibleLayerOnSurface = true;
            if (!layer->SupportsGpuExclusivePresent()) {
                gpuExclusiveEligible = false;
                break;
            }
        }
    }
    bool presented = false;
    std::string presentDetail = "cpu_present_path";
    if (dawnBackendSelected && useLayeredSurfaces_) {
        presentDetail = EvaluateGpuFinalPresentPolicy().detail;
    }
    if (wantsGpuPresent && !hasVisibleLayerOnSurface) {
        if (IsWindowVisible(surface.hwnd)) {
            ShowWindow(surface.hwnd, SW_HIDE);
        }
        surface.hadVisibleContent = false;
        surface.hadPresentedFrame = false;
        presentDetail = "gpu_present_surface_hidden_no_visible_layers";
        {
            std::lock_guard<std::mutex> lock(gpuPresentDetailMutex_);
            gpuPresentLastDetail_ = presentDetail;
        }
        return;
    }
    if (wantsGpuPresent) {
        gpuPresentAttemptCount_.fetch_add(1, std::memory_order_relaxed);
        presented = dawnPresenter_.Present(frame);
        presentDetail = dawnPresenter_.LastDetail();
        if (presented && !gpuExclusiveEligible) {
            presented = false;
            presentDetail += "_fallback_nonexclusive_layer_visible";
        }
        if (presented) {
            gpuPresentSuccessCount_.fetch_add(1, std::memory_order_relaxed);
            gpuPresentConsecutiveFailures_.store(0, std::memory_order_release);
            surface.hadPresentedFrame = true;
            surface.hadVisibleContent = hasVisibleLayerOnSurface;
            if (!IsWindowVisible(surface.hwnd)) {
                ShowWindow(surface.hwnd, SW_SHOWNA);
            }
        } else {
            gpuPresentFallbackCount_.fetch_add(1, std::memory_order_relaxed);
            surface.hadPresentedFrame = false;
            surface.hadVisibleContent = false;
            if (IsWindowVisible(surface.hwnd)) {
                ShowWindow(surface.hwnd, SW_HIDE);
            }
            const uint32_t failures =
                gpuPresentConsecutiveFailures_.fetch_add(1, std::memory_order_acq_rel) + 1;
            const bool neverSucceededYet =
                (gpuPresentSuccessCount_.load(std::memory_order_acquire) == 0);
            if (failures >= kGpuPresentRollbackFailureThreshold ||
                (neverSucceededYet && failures >= 1)) {
                layeredCpuFallbackUntilMs_.store(
                    NowMs() + kGpuPresentRollbackCooldownMs,
                    std::memory_order_release);
                pendingLayeredRollback_.store(true, std::memory_order_release);
                if (presentDetail.empty()) {
                    presentDetail = "gpu_present_failed";
                }
                presentDetail += neverSucceededYet
                    ? "_startup_fail_fast_rollback_layered_cpu_cooldown"
                    : "_auto_rollback_layered_cpu_cooldown";
            }
        }
    }
    if (!presented) {
        if (useLayeredSurfaces_ || !wantsGpuPresent) {
            cpuPresenter_.Present(frame);
            if (wantsGpuPresent && presentDetail.empty()) {
                presentDetail = "gpu_present_fallback_cpu";
            }
        } else if (presentDetail.empty()) {
            presentDetail = "gpu_present_failed_nonlayered";
        }
    }
    {
        std::lock_guard<std::mutex> lock(gpuPresentDetailMutex_);
        gpuPresentLastDetail_ = presentDetail;
    }
}

void OverlayHostWindow::CollectGpuCommandStream(uint64_t nowMs) {
    gpuCommandStream_.Reset(nowMs);
    gpuCommandStream_.Reserve(layers_.size());
    for (const auto& layer : layers_) {
        if (layer && layer->IsAlive()) {
            layer->AppendGpuCommands(gpuCommandStream_, nowMs);
        }
    }

    uint32_t trailCount = 0;
    uint32_t rippleCount = 0;
    uint32_t particleCount = 0;
    uint32_t rippleHoldCount = 0;
    for (const auto& cmd : gpuCommandStream_.Commands()) {
        switch (cmd.type) {
        case gpu::OverlayGpuCommandType::TrailPolyline:
            ++trailCount;
            break;
        case gpu::OverlayGpuCommandType::RipplePulse:
            ++rippleCount;
            if ((cmd.flags & gpu::OverlayGpuCommandFlags::kHoldContinuous) != 0) {
                ++rippleHoldCount;
            }
            break;
        case gpu::OverlayGpuCommandType::ParticleSprites:
            ++particleCount;
            break;
        default:
            break;
        }
    }
    gpuCommandFrameTickMs_.store(nowMs, std::memory_order_release);
    gpuCommandCount_.store((uint32_t)gpuCommandStream_.Commands().size(), std::memory_order_release);
    gpuTrailCommandCount_.store(trailCount, std::memory_order_release);
    gpuRippleCommandCount_.store(rippleCount, std::memory_order_release);
    gpuParticleCommandCount_.store(particleCount, std::memory_order_release);
    gpuRippleHoldCommandCount_.store(rippleHoldCount, std::memory_order_release);
    if (!gpuCommandStream_.Commands().empty()) {
        lastNonEmptyGpuCommandTickMs_.store(nowMs, std::memory_order_release);
    }

    const gpu::DawnRuntimeStatus runtime = gpu::GetDawnRuntimeStatusFast();
    const gpu::DawnOverlayBridgeStatus bridge = gpu::GetDawnOverlayBridgeStatus();
    gpu::SubmitOverlayGpuCommands(
        gpuCommandStream_,
        runtime,
        bridge,
        gpuSubmitActiveBackend_,
        gpuSubmitPipelineMode_);
}

void OverlayHostWindow::UpdateFrameLoopTimerInterval(UINT intervalMs) {
    if (!ticking_ || !timerHwnd_) return;
    const UINT safeIntervalMs = intervalMs == 0 ? kFrameTimerIntervalDefaultMs : intervalMs;
    if (currentTimerIntervalMs_ == safeIntervalMs) return;
    currentTimerIntervalMs_ = safeIntervalMs;
    KillTimer(timerHwnd_, kTimerId);
    SetTimer(timerHwnd_, kTimerId, currentTimerIntervalMs_, nullptr);
}

UINT OverlayHostWindow::ResolveNextTimerIntervalMs() const {
    const uint32_t holdCount = gpuRippleHoldCommandCount_.load(std::memory_order_acquire);
    if (holdCount > 0) return kFrameTimerIntervalHoldMs;
    if (gpuSubmitActiveBackend_ == "dawn") return kFrameTimerIntervalDawnActiveMs;
    return kFrameTimerIntervalDefaultMs;
}

gpu::GpuFinalPresentPolicyDecision OverlayHostWindow::EvaluateGpuFinalPresentPolicy() const {
    gpu::GpuFinalPresentPolicyInput in{};
    in.optInEnabled = gpu::IsGpuFinalPresentOptInEnabled();
    const uint64_t nowMs = GetTickCount64();
    const uint64_t fallbackUntilMs = layeredCpuFallbackUntilMs_.load(std::memory_order_acquire);
    in.forceLayeredCpuFallback = (fallbackUntilMs != 0 && nowMs < fallbackUntilMs);
    in.activeBackend = gpuSubmitActiveBackend_;
    in.pipelineMode = gpuSubmitPipelineMode_;

    uint32_t layerCount = 0;
    bool allGpuExclusive = true;
    for (const auto& layer : layers_) {
        if (!layer || !layer->IsAlive()) continue;
        ++layerCount;
        if (!layer->SupportsGpuExclusivePresent()) {
            allGpuExclusive = false;
            break;
        }
    }
    in.activeLayerCount = layerCount;
    in.allLayersGpuExclusive = allGpuExclusive;

    const gpu::GpuFinalPresentCapability cap = gpu::GetGpuFinalPresentCapability();
    in.runtimeCapabilityLikelyAvailable = cap.likelyAvailable;

    in.processUptimeMs = (nowMs >= g_processStartTickMs) ? (nowMs - g_processStartTickMs) : 0;
    const uint64_t lastNonEmptyMs = lastNonEmptyGpuCommandTickMs_.load(std::memory_order_acquire);
    in.hasRecentGpuCommandActivity = (lastNonEmptyMs != 0) && (nowMs - lastNonEmptyMs <= 1200);
    const bool canProbeHostChain =
        in.optInEnabled &&
        !in.forceLayeredCpuFallback &&
        in.activeBackend == "dawn" &&
        in.pipelineMode == "dawn_compositor" &&
        in.activeLayerCount > 0 &&
        in.allLayersGpuExclusive &&
        in.runtimeCapabilityLikelyAvailable &&
        in.processUptimeMs >= 3000 &&
        in.hasRecentGpuCommandActivity;
    if (canProbeHostChain) {
        const gpu::GpuFinalPresentHostChainStatus hostChain = gpu::GetGpuFinalPresentHostChainStatus(false);
        in.hostChainActive = hostChain.active;
    }
    gpu::GpuFinalPresentTakeoverGateInput takeoverGateIn{};
    takeoverGateIn.optInEnabled = in.optInEnabled;
    takeoverGateIn.runtimeCapabilityLikelyAvailable = in.runtimeCapabilityLikelyAvailable;
    takeoverGateIn.hostChainActive = in.hostChainActive;
    in.hostChainTakeoverReady = gpu::GetGpuFinalPresentTakeoverGateStatus(takeoverGateIn, false).ready;

    return gpu::ResolveGpuFinalPresentPolicy(in);
}

bool OverlayHostWindow::ShouldUseLayeredSurfaces() const {
    const gpu::GpuFinalPresentPolicyDecision decision = EvaluateGpuFinalPresentPolicy();
    return decision.useLayeredSurfaces;
}

bool OverlayHostWindow::EnsureSurfaceMode(bool forceRebuild) {
    const bool desiredLayered = ShouldUseLayeredSurfaces();
    if (!forceRebuild && desiredLayered == useLayeredSurfaces_) {
        return true;
    }
    useLayeredSurfaces_ = desiredLayered;
    if (surfaces_.empty()) {
        return true;
    }
    return RebuildSurfaces();
}

bool OverlayHostWindow::RebuildSurfaces() {
    const std::vector<ScreenRect> desired = QueryMonitorRects();
    if (desired.empty()) return false;

    const bool wasTicking = ticking_;
    if (wasTicking) StopFrameLoop();
    DestroySurfaces();

    for (const auto& r : desired) {
        HostSurface surface{};
        DWORD ex = WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
        if (useLayeredSurfaces_) {
            ex |= WS_EX_LAYERED;
        }
        surface.hwnd = CreateWindowExW(
            ex,
            ClassName(),
            L"",
            WS_POPUP,
            r.x, r.y, r.w, r.h,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            this);
        if (!surface.hwnd) {
            DestroySurfaces();
            return false;
        }

        RECT rcClient{};
        int cw = r.w;
        int ch = r.h;
        if (GetClientRect(surface.hwnd, &rcClient)) {
            cw = rcClient.right - rcClient.left;
            ch = rcClient.bottom - rcClient.top;
        }
        if (!EnsureSurfaceBuffer(surface, cw, ch)) {
            DestroySurfaces();
            return false;
        }
        ShowWindow(surface.hwnd, SW_HIDE);
        RefreshSurfaceRect(surface);
        surfaces_.push_back(std::move(surface));
    }

    timerHwnd_ = surfaces_.empty() ? nullptr : surfaces_[0].hwnd;
    SetOverlayWindowHandle(timerHwnd_);

    UnionRects(surfaces_, &virtualX_, &virtualY_, &virtualW_, &virtualH_);
    SetOverlayOriginOverride(virtualX_, virtualY_);

    if (wasTicking) StartFrameLoop();
    return timerHwnd_ != nullptr;
}

void OverlayHostWindow::DestroySurfaces() {
    for (auto& surface : surfaces_) {
        if (surface.dib) {
            DeleteObject(surface.dib);
            surface.dib = nullptr;
        }
        if (surface.memDc) {
            DeleteDC(surface.memDc);
            surface.memDc = nullptr;
        }
        surface.bits = nullptr;
        if (surface.hwnd) {
            DestroyWindow(surface.hwnd);
            surface.hwnd = nullptr;
        }
        surface.width = 0;
        surface.height = 0;
        surface.hadVisibleContent = false;
        surface.hadPresentedFrame = false;
    }
    surfaces_.clear();
    timerHwnd_ = nullptr;
    virtualX_ = 0;
    virtualY_ = 0;
    virtualW_ = 0;
    virtualH_ = 0;
}

void OverlayHostWindow::SyncBoundsWithVirtualScreen(bool forceMove) {
    if (surfaces_.empty()) return;

    const std::vector<ScreenRect> desired = QueryMonitorRects();
    if (desired.empty()) return;
    if (desired.size() != surfaces_.size()) {
        RebuildSurfaces();
        return;
    }

    bool mismatch = forceMove;
    for (size_t i = 0; i < desired.size(); ++i) {
        const auto& s = surfaces_[i];
        const auto& d = desired[i];
        if (s.x != d.x || s.y != d.y || s.width != d.w || s.height != d.h) {
            mismatch = true;
            break;
        }
    }
    if (mismatch) {
        RebuildSurfaces();
        return;
    }

    for (auto& surface : surfaces_) {
        RefreshSurfaceRect(surface);
    }
    UnionRects(surfaces_, &virtualX_, &virtualY_, &virtualW_, &virtualH_);
    SetOverlayOriginOverride(virtualX_, virtualY_);
}

void OverlayHostWindow::StartFrameLoop() {
    if (surfaces_.empty()) {
        if (!RebuildSurfaces()) return;
    }
    if (!timerHwnd_) return;
    if (ticking_) return;
    ticking_ = true;
    if (!timerResolutionRaised_) {
        const MMRESULT mm = timeBeginPeriod(1);
        timerResolutionRaised_ = (mm == TIMERR_NOERROR);
    }
    SyncBoundsWithVirtualScreen(false);
    // Warm a first frame before showing windows to avoid startup black flash.
    const uint64_t nowMs = NowMs();
    for (auto& layer : layers_) {
        if (layer) {
            layer->Update(nowMs);
        }
    }
    CollectGpuCommandStream(nowMs);
    Render();
    for (auto& surface : surfaces_) {
        if (!surface.hwnd) continue;
        if (useLayeredSurfaces_) {
            ShowWindow(surface.hwnd, SW_SHOWNA);
            continue;
        }
        if (surface.hadPresentedFrame && surface.hadVisibleContent) {
            ShowWindow(surface.hwnd, SW_SHOWNA);
        } else {
            ShowWindow(surface.hwnd, SW_HIDE);
        }
    }
    currentTimerIntervalMs_ = kFrameTimerIntervalDefaultMs;
    SetTimer(timerHwnd_, kTimerId, currentTimerIntervalMs_, nullptr);
    EnsureTopmostZOrder(true);
}

void OverlayHostWindow::StopFrameLoop() {
    if (!ticking_) return;
    ticking_ = false;
    if (timerHwnd_) {
        KillTimer(timerHwnd_, kTimerId);
    }
    currentTimerIntervalMs_ = 0;
    gpuRippleHoldCommandCount_.store(0, std::memory_order_release);
    immediateFrameKickPending_.store(false, std::memory_order_release);
    lastImmediateFrameKickMs_.store(0, std::memory_order_release);
    if (timerResolutionRaised_) {
        timeEndPeriod(1);
        timerResolutionRaised_ = false;
    }
    for (auto& surface : surfaces_) {
        if (surface.hwnd) ShowWindow(surface.hwnd, SW_HIDE);
    }
}

void OverlayHostWindow::EnsureTopmostZOrder(bool force) {
    if (surfaces_.empty()) return;
    const uint64_t now = NowMs();
    if (!force && (now - lastTopmostEnsureMs_ < kTopmostReassertIntervalMs)) return;
    lastTopmostEnsureMs_ = now;
    for (auto& surface : surfaces_) {
        if (!surface.hwnd) continue;
        SetWindowPos(surface.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
}

void CALLBACK OverlayHostWindow::ForegroundEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    OverlayHostWindow* self = g_overlayForegroundHookOwner;
    if (!self || !self->timerHwnd_) return;
    if (!IsWindow(self->timerHwnd_)) return;
    if (hwnd == self->timerHwnd_) return;
    PostMessageW(self->timerHwnd_, kMsgEnsureTopmost, 0, 0);
}

void OverlayHostWindow::RegisterForegroundHook() {
    if (foregroundHook_) return;
    foregroundHook_ = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND,
        nullptr,
        &OverlayHostWindow::ForegroundEventProc,
        0,
        0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    if (foregroundHook_) {
        g_overlayForegroundHookOwner = this;
    }
}

void OverlayHostWindow::UnregisterForegroundHook() {
    if (foregroundHook_) {
        UnhookWinEvent(foregroundHook_);
        foregroundHook_ = nullptr;
    }
    if (g_overlayForegroundHookOwner == this) {
        g_overlayForegroundHookOwner = nullptr;
    }
}

} // namespace mousefx
