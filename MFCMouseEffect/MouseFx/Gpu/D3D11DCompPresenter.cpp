#include "pch.h"

#include "D3D11DCompPresenter.h"
#include "GpuTakeoverControl.h"

#include <fstream>
#include <iterator>
#include <sstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")

namespace mousefx::gpu {

namespace {
const char* ResolveTakeoverFastGuardReasonUnlocked(
    const D3D11DCompPresenterStatus& status,
    bool takeoverAttempted,
    bool holdNeon3dActive) {
    if (!status.initialized) return "takeover_not_initialized";
    if (!status.takeoverEnabled) return "takeover_disabled";
    if (!status.takeoverEligible) return "takeover_not_eligible";
    if (status.takeoverActive) return "takeover_already_active";
    if (takeoverAttempted) return "takeover_already_attempted";
    if (status.visibleTrialEnabled && !holdNeon3dActive) {
        return "takeover_wait_hold_neon3d_active";
    }
    return nullptr;
}

bool IsVisibleTrialFallbackLayered(const D3D11DCompPresenterStatus& status) {
    return status.takeoverControl == "runtime_auto_off" &&
           status.takeoverControlDetail == "visible_trial_ready_fallback_layered";
}

std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char ch : s) {
        switch (ch) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out.push_back(ch); break;
        }
    }
    return out;
}

void WriteTrialResultSnapshot(const D3D11DCompPresenterStatus& status) {
    const std::filesystem::path diagDir = ResolveGpuDiagDirFromCurrentModule();
    if (diagDir.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;

    const std::filesystem::path file = diagDir / L"gpu_takeover_trial_result_auto.json";
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;
    out
        << "{"
        << "\"last_trial_tick_ms\":" << status.lastTrialTickMs << ","
        << "\"last_trial_result\":\"" << JsonEscape(status.lastTrialResult) << "\","
        << "\"detail\":\"" << JsonEscape(status.detail) << "\","
        << "\"takeover_control\":\"" << JsonEscape(status.takeoverControl) << "\","
        << "\"takeover_control_detail\":\"" << JsonEscape(status.takeoverControlDetail) << "\","
        << "\"takeover_attempts\":" << status.takeoverAttempts << ","
        << "\"takeover_fallbacks\":" << status.takeoverFallbacks
        << "}";
}
} // namespace

const wchar_t* D3D11DCompPresenter::ProbeClassName() {
    return L"MouseFxDCompProbeWindow";
}

bool D3D11DCompPresenter::EnsureProbeClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &D3D11DCompPresenter::ProbeWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = ProbeClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

LRESULT CALLBACK D3D11DCompPresenter::ProbeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool D3D11DCompPresenter::CreateProbeWindowAndTarget() {
    DestroyProbeWindowAndTarget();
    if (!dcompDevice_) return false;
    if (!EnsureProbeClassRegistered()) {
        status_.detail = "probe_class_register_failed";
        return false;
    }

    probeHwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        ProbeClassName(),
        L"",
        WS_POPUP,
        0,
        0,
        1,
        1,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);
    if (!probeHwnd_) {
        status_.detail = "probe_window_create_failed";
        return false;
    }

    HRESULT hr = dcompDevice_->CreateTargetForHwnd(probeHwnd_, TRUE, dcompTarget_.GetAddressOf());
    if (FAILED(hr) || !dcompTarget_) {
        status_.detail = "dcomp_create_target_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }

    hr = dcompDevice_->CreateVisual(dcompRootVisual_.GetAddressOf());
    if (FAILED(hr) || !dcompRootVisual_) {
        status_.detail = "dcomp_create_visual_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }

    hr = dcompTarget_->SetRoot(dcompRootVisual_.Get());
    if (FAILED(hr)) {
        status_.detail = "dcomp_set_root_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }

    hr = dcompDevice_->Commit();
    if (FAILED(hr)) {
        status_.detail = "dcomp_commit_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }
    return true;
}

bool D3D11DCompPresenter::CreateProbeCompositionSwapChain() {
    if (!dxgiDevice_ || !dcompRootVisual_ || !dcompDevice_) {
        status_.detail = "takeover_swapchain_missing_prerequisites";
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    HRESULT hr = dxgiDevice_->GetAdapter(adapter.GetAddressOf());
    if (FAILED(hr) || !adapter) {
        status_.detail = "takeover_swapchain_get_adapter_failed";
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
    hr = adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory2.GetAddressOf()));
    if (FAILED(hr) || !factory2) {
        status_.detail = "takeover_swapchain_get_factory2_failed";
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = 1;
    desc.Height = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    desc.Flags = 0;

    compositionSwapChain_.Reset();
    hr = factory2->CreateSwapChainForComposition(
        d3d11Device_.Get(),
        &desc,
        nullptr,
        compositionSwapChain_.GetAddressOf());
    if (FAILED(hr) || !compositionSwapChain_) {
        status_.detail = "takeover_swapchain_create_failed";
        return false;
    }

    hr = dcompRootVisual_->SetContent(compositionSwapChain_.Get());
    if (FAILED(hr)) {
        status_.detail = "takeover_swapchain_set_content_failed";
        compositionSwapChain_.Reset();
        return false;
    }

    hr = dcompDevice_->Commit();
    if (FAILED(hr)) {
        status_.detail = "takeover_swapchain_commit_failed";
        compositionSwapChain_.Reset();
        return false;
    }

    // Probe present once with transparent frame to validate end-to-end composition chain.
    hr = compositionSwapChain_->Present(0, 0);
    if (FAILED(hr)) {
        status_.detail = "takeover_swapchain_present_failed";
        compositionSwapChain_.Reset();
        return false;
    }
    status_.compositionSwapChainReady = true;
    compositionWidth_ = 1;
    compositionHeight_ = 1;
    return true;
}

bool D3D11DCompPresenter::TryPrepareVisibleTrialTarget() {
    if (!status_.visibleTrialEnabled) return false;
    if (!visibleTrialHwnd_ || !IsWindow(visibleTrialHwnd_)) {
        status_.detail = "visible_trial_missing_hwnd";
        return false;
    }
    if (!dcompDevice_ || !compositionSwapChain_) {
        status_.detail = "visible_trial_missing_prerequisites";
        return false;
    }

    visibleTrialRootVisual_.Reset();
    visibleTrialTarget_.Reset();

    HRESULT hr = dcompDevice_->CreateTargetForHwnd(visibleTrialHwnd_, TRUE, visibleTrialTarget_.GetAddressOf());
    if (FAILED(hr) || !visibleTrialTarget_) {
        status_.detail = "visible_trial_create_target_failed";
        return false;
    }

    hr = dcompDevice_->CreateVisual(visibleTrialRootVisual_.GetAddressOf());
    if (FAILED(hr) || !visibleTrialRootVisual_) {
        status_.detail = "visible_trial_create_visual_failed";
        visibleTrialTarget_.Reset();
        return false;
    }

    hr = visibleTrialRootVisual_->SetContent(compositionSwapChain_.Get());
    if (FAILED(hr)) {
        status_.detail = "visible_trial_set_content_failed";
        visibleTrialRootVisual_.Reset();
        visibleTrialTarget_.Reset();
        return false;
    }

    hr = visibleTrialTarget_->SetRoot(visibleTrialRootVisual_.Get());
    if (FAILED(hr)) {
        status_.detail = "visible_trial_set_root_failed";
        visibleTrialRootVisual_.Reset();
        visibleTrialTarget_.Reset();
        return false;
    }

    hr = dcompDevice_->Commit();
    if (FAILED(hr)) {
        status_.detail = "visible_trial_commit_failed";
        visibleTrialRootVisual_.Reset();
        visibleTrialTarget_.Reset();
        return false;
    }

    hr = compositionSwapChain_->Present(0, 0);
    if (FAILED(hr)) {
        status_.detail = "visible_trial_present_failed";
        visibleTrialRootVisual_.Reset();
        visibleTrialTarget_.Reset();
        return false;
    }

    status_.visibleTrialReady = true;
    return true;
}

void D3D11DCompPresenter::DestroyProbeWindowAndTarget() {
    visibleTrialRootVisual_.Reset();
    visibleTrialTarget_.Reset();
    compositionSwapChain_.Reset();
    compositionWidth_ = 0;
    compositionHeight_ = 0;
    dcompRootVisual_.Reset();
    dcompTarget_.Reset();
    if (probeHwnd_) {
        DestroyWindow(probeHwnd_);
        probeHwnd_ = nullptr;
    }
}

void D3D11DCompPresenter::SetVisibleTrialHwnd(HWND hwnd) {
    std::lock_guard<std::mutex> lock(mutex_);
    visibleTrialHwnd_ = hwnd;
}

bool D3D11DCompPresenter::TryActivateTakeoverPath() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!status_.initialized) {
        status_.lastTrialTickMs = GetTickCount64();
        status_.lastTrialResult = "skip_not_initialized";
        status_.detail = "takeover_skipped_not_initialized";
        WriteTrialResultSnapshot(status_);
        return false;
    }
    if (status_.takeoverActive) {
        status_.lastTrialTickMs = GetTickCount64();
        status_.lastTrialResult = "already_active";
        WriteTrialResultSnapshot(status_);
        return true;
    }
    if (!status_.takeoverEnabled) {
        status_.lastTrialTickMs = GetTickCount64();
        status_.lastTrialResult = "skip_disabled";
        status_.detail = "takeover_disabled";
        WriteTrialResultSnapshot(status_);
        return false;
    }
    if (!status_.takeoverEligible) {
        status_.lastTrialTickMs = GetTickCount64();
        status_.lastTrialResult = "skip_not_eligible";
        status_.detail = "takeover_not_eligible";
        WriteTrialResultSnapshot(status_);
        return false;
    }
    if (takeoverAttempted_) {
        status_.lastTrialTickMs = GetTickCount64();
        status_.lastTrialResult = "skip_already_attempted";
        WriteTrialResultSnapshot(status_);
        return false;
    }

    if (status_.takeoverControl == "file_once") {
        status_.controlOnceFileConsumed = ConsumeOneShotControlFileForSource("file_once");
    } else if (status_.takeoverControl == "file_visible_trial_once" ||
               status_.takeoverControl == "file_visible_trial_once_downgraded") {
        status_.controlVisibleTrialOnceFileConsumed = ConsumeOneShotControlFileForSource("file_visible_trial_once");
    }

    takeoverAttempted_ = true;
    status_.lastTrialTickMs = GetTickCount64();
    status_.lastTrialResult = "attempt_started";
    status_.takeoverAttempts += 1;

    if (!CreateProbeCompositionSwapChain()) {
        status_.takeoverFallbacks += 1;
        status_.takeoverActive = false;
        status_.takeoverEnabled = false;
        status_.takeoverControl = "runtime_auto_off";
        status_.takeoverControlDetail = "takeover_trial_swapchain_create_failed";
        status_.lastTrialResult = "swapchain_create_failed";
        WriteGpuAutoDisableMarker("takeover_trial_swapchain_create_failed");
        WriteTrialResultSnapshot(status_);
        return false;
    }

    if (status_.visibleTrialEnabled) {
        if (!TryPrepareVisibleTrialTarget()) {
            status_.takeoverFallbacks += 1;
            status_.takeoverActive = false;
            status_.takeoverEnabled = false;
            status_.takeoverControl = "runtime_auto_off";
            status_.takeoverControlDetail = "visible_trial_prepare_failed";
            status_.lastTrialResult = "visible_trial_prepare_failed";
            WriteGpuAutoDisableMarker("visible_trial_prepare_failed");
            WriteTrialResultSnapshot(status_);
            return false;
        }
    }

    // Stage-11 safety policy:
    // takeover chain is proven on hidden probe path, but visible layered present remains authoritative.
    status_.takeoverFallbacks += 1;
    status_.takeoverActive = false;
    status_.takeoverEnabled = false;
    status_.takeoverControl = "runtime_auto_off";
    if (status_.visibleTrialEnabled) {
        status_.takeoverControlDetail = "visible_trial_ready_fallback_layered";
        status_.detail = "visible_trial_ready_fallback_layered";
        status_.lastTrialResult = "visible_trial_ready_fallback_layered";
        WriteGpuAutoDisableMarker("visible_trial_ready_fallback_layered");
    } else {
        status_.takeoverControlDetail = "takeover_trial_swapchain_ready_fallback_layered";
        status_.detail = "takeover_trial_swapchain_ready_fallback_layered";
        status_.lastTrialResult = "probe_swapchain_ready_fallback_layered";
        WriteGpuAutoDisableMarker("takeover_trial_swapchain_ready_fallback_layered");
    }
    WriteTrialResultSnapshot(status_);
    return false;
}

bool D3D11DCompPresenter::SubmitTrialFrameBGRAUnlocked(const void* pixels, int width, int height, int strideBytes) {
    if (!status_.visibleTrialEnabled) {
        status_.trialFrameSubmitSkippedDisabled += 1;
        status_.detail = "trial_frame_skip_visible_trial_disabled";
        return false;
    }
    if (!status_.visibleTrialReady || !compositionSwapChain_ || !d3d11Context_) {
        status_.trialFrameSubmitSkippedNotReady += 1;
        status_.detail = "trial_frame_skip_visible_trial_not_ready";
        return false;
    }
    status_.trialFrameSubmitAttempts += 1;

    if (!pixels || width <= 0 || height <= 0 || strideBytes < (width * 4)) {
        status_.trialFrameSubmitFailure += 1;
        status_.detail = "trial_frame_invalid_args";
        return false;
    }

    if (compositionWidth_ != width || compositionHeight_ != height) {
        HRESULT hrResize = compositionSwapChain_->ResizeBuffers(
            0,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            DXGI_FORMAT_B8G8R8A8_UNORM,
            0);
        if (FAILED(hrResize)) {
            status_.trialFrameSubmitFailure += 1;
            status_.detail = "trial_frame_resize_buffers_failed";
            return false;
        }
        compositionWidth_ = width;
        compositionHeight_ = height;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = compositionSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr) || !backBuffer) {
        status_.trialFrameSubmitFailure += 1;
        status_.detail = "trial_frame_get_backbuffer_failed";
        return false;
    }

    const D3D11_BOX fullRegion = {
        0u,
        0u,
        0u,
        static_cast<UINT>(width),
        static_cast<UINT>(height),
        1u
    };
    d3d11Context_->UpdateSubresource(
        backBuffer.Get(),
        0,
        &fullRegion,
        pixels,
        static_cast<UINT>(strideBytes),
        0);

    hr = compositionSwapChain_->Present(0, 0);
    if (FAILED(hr)) {
        status_.trialFrameSubmitFailure += 1;
        status_.detail = "trial_frame_present_failed";
        return false;
    }

    status_.trialFrameSubmitSuccess += 1;
    status_.detail = "trial_frame_submit_ok";
    return true;
}

bool D3D11DCompPresenter::SubmitTrialFrameBGRAIfEnabled(const void* pixels, int width, int height, int strideBytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    const bool allowedByPolicy = (status_.visibleTrialEnabled && status_.takeoverEnabled) ||
                                 (status_.visibleTrialReady && IsVisibleTrialFallbackLayered(status_));
    status_.trialFrameUploadEnabled = allowedByPolicy;
    if (!allowedByPolicy) {
        return false;
    }
    return SubmitTrialFrameBGRAUnlocked(pixels, width, height, strideBytes);
}

bool D3D11DCompPresenter::SubmitTrialFrameBGRA(const void* pixels, int width, int height, int strideBytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    return SubmitTrialFrameBGRAUnlocked(pixels, width, height, strideBytes);
}

bool D3D11DCompPresenter::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_.initialized) {
        return true;
    }

    status_ = D3D11DCompPresenterStatus{};
    d3d11Device_.Reset();
    d3d11Context_.Reset();
    dxgiDevice_.Reset();
    dcompDevice_.Reset();
    dcompTarget_.Reset();
    dcompRootVisual_.Reset();
    compositionSwapChain_.Reset();
    compositionWidth_ = 0;
    compositionHeight_ = 0;
    probeHwnd_ = nullptr;
    visibleTrialHwnd_ = nullptr;
    takeoverAttempted_ = false;
    const TakeoverControlDecision control = ResolveTakeoverControlDecision();
    status_.visibleTrialEnabled = control.visibleTrialEnabled;
    status_.rearmProcessed = control.rearmProcessed;
    status_.controlOnFilePresent = control.onFilePresent;
    status_.controlOffFilePresent = control.offFilePresent;
    status_.controlAutoOffFilePresent = control.autoOffFilePresent;
    status_.controlVisibleTrialFilePresent = control.visibleTrialFilePresent;
    status_.controlOnceFilePresent = control.onceFilePresent;
    status_.controlOnceFileConsumed = control.onceFileConsumed;
    status_.controlVisibleTrialOnceFilePresent = control.visibleTrialOnceFilePresent;
    status_.controlVisibleTrialOnceFileConsumed = control.visibleTrialOnceFileConsumed;
    status_.controlVisibleTrialDowngradedByMultiMonitor = control.visibleTrialDowngradedByMultiMonitor;
    status_.takeoverEnabled = control.takeoverEnabled;
    status_.trialFrameUploadEnabled = false;
    status_.takeoverControl = control.source;
    status_.takeoverControlDetail = control.detail;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    const UINT featureLevelCount = static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0]));
    D3D_FEATURE_LEVEL chosenLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        featureLevelCount,
        D3D11_SDK_VERSION,
        d3d11Device_.GetAddressOf(),
        &chosenLevel,
        d3d11Context_.GetAddressOf());
    if (FAILED(hr)) {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            flags,
            featureLevels,
            featureLevelCount,
            D3D11_SDK_VERSION,
            d3d11Device_.GetAddressOf(),
            &chosenLevel,
            d3d11Context_.GetAddressOf());
    }
    (void)chosenLevel;
    if (FAILED(hr) || !d3d11Device_ || !d3d11Context_) {
        status_.detail = "d3d11_create_device_failed";
        return false;
    }
    status_.d3d11DeviceReady = true;

    hr = d3d11Device_.As(&dxgiDevice_);
    if (FAILED(hr) || !dxgiDevice_) {
        status_.detail = "dxgi_device_query_failed";
        return false;
    }

    hr = DCompositionCreateDevice(
        dxgiDevice_.Get(),
        __uuidof(IDCompositionDevice),
        reinterpret_cast<void**>(dcompDevice_.GetAddressOf()));
    if (FAILED(hr) || !dcompDevice_) {
        status_.detail = "dcomp_create_device_failed";
        return false;
    }

    status_.initialized = true;
    status_.dcompDeviceReady = true;
    status_.detail = "d3d11_dcomp_device_ready";

    if (!CreateProbeWindowAndTarget()) {
        return false;
    }
    status_.dcompTargetReady = true;
    status_.takeoverEligible = status_.d3d11DeviceReady && status_.dcompDeviceReady && status_.dcompTargetReady;
    status_.detail = status_.takeoverEnabled ? "dcomp_probe_ready_takeover_enabled" : "dcomp_probe_ready_takeover_disabled";
    return true;
}

void D3D11DCompPresenter::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    DestroyProbeWindowAndTarget();
    dcompDevice_.Reset();
    dxgiDevice_.Reset();
    d3d11Context_.Reset();
    d3d11Device_.Reset();
    takeoverAttempted_ = false;
    status_ = D3D11DCompPresenterStatus{};
    status_.detail = "shutdown";
}

D3D11DCompPresenterStatus D3D11DCompPresenter::GetStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

bool D3D11DCompPresenter::ShouldAttemptTakeover(bool holdNeon3dActive) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ResolveTakeoverFastGuardReasonUnlocked(status_, takeoverAttempted_, holdNeon3dActive) == nullptr;
}

std::string D3D11DCompPresenter::GetTakeoverFastGuardReason(bool holdNeon3dActive) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const char* reason = ResolveTakeoverFastGuardReasonUnlocked(status_, takeoverAttempted_, holdNeon3dActive);
    return reason ? reason : "takeover_fast_guard_open";
}

void D3D11DCompPresenter::RecordTakeoverNotAttempted(const char* reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    status_.lastTrialTickMs = GetTickCount64();
    status_.lastTrialResult = "not_attempted";
    if (reason && *reason) {
        status_.detail = reason;
    }
    WriteTrialResultSnapshot(status_);
}

bool D3D11DCompPresenter::IsTrialFrameUploadEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (status_.visibleTrialEnabled && status_.takeoverEnabled) ||
           (status_.visibleTrialReady && IsVisibleTrialFallbackLayered(status_));
}

} // namespace mousefx::gpu
