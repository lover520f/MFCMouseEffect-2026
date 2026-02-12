#include "pch.h"

#include "OverlayHostWindow.h"
#include "MouseFx/Core/OverlayCoordSpace.h"

#include <algorithm>
#include <vector>

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
static OverlayHostWindow* g_overlayForegroundHookOwner = nullptr;

} // namespace

OverlayHostWindow::OverlayHostWindow() = default;

OverlayHostWindow::~OverlayHostWindow() {
    Shutdown();
}

gpu::D3D11DCompPresenterStatus OverlayHostWindow::GetGpuPresentHostStatus() const {
    return d3d11DcompPresenter_.GetStatus();
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
    InitializeGpuPresentHost();
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
    ShutdownGpuPresentHost();
    layers_.clear();
    ClearOverlayWindowHandle();
    ClearOverlayOriginOverride();
}

void OverlayHostWindow::InitializeGpuPresentHost() {
    // Stage-2: host-chain device bring-up is best-effort and does not affect current layered path.
    (void)d3d11DcompPresenter_.Initialize();
}

void OverlayHostWindow::ShutdownGpuPresentHost() {
    d3d11DcompPresenter_.Shutdown();
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
    Render();
}

void OverlayHostWindow::Render() {
    for (auto& surface : surfaces_) {
        RenderSurface(surface);
    }
}

void OverlayHostWindow::RenderSurface(HostSurface& surface) {
    if (!surface.hwnd || !surface.memDc || !surface.bits || surface.width <= 0 || surface.height <= 0) return;

    SetOverlayOriginOverride(surface.x, surface.y);
    ZeroMemory(surface.bits, (size_t)surface.width * (size_t)surface.height * 4);

    Gdiplus::Bitmap bmp(surface.width, surface.height, surface.width * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(surface.bits));
    Gdiplus::Graphics graphics(&bmp);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    for (auto& layer : layers_) {
        if (layer && layer->IsAlive()) {
            layer->Render(graphics);
        }
    }

    POINT ptSrc{0, 0};
    SIZE sizeWnd{surface.width, surface.height};
    POINT ptDst{surface.x, surface.y};
    BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(surface.hwnd, nullptr, &ptDst, &sizeWnd, surface.memDc, &ptSrc, 0, &bf, ULW_ALPHA);
}

bool OverlayHostWindow::RebuildSurfaces() {
    const std::vector<ScreenRect> desired = QueryMonitorRects();
    if (desired.empty()) return false;

    const bool wasTicking = ticking_;
    if (wasTicking) StopFrameLoop();
    DestroySurfaces();

    for (const auto& r : desired) {
        HostSurface surface{};
        const DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
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
    (void)d3d11DcompPresenter_.TryActivateTakeoverPath();
    SyncBoundsWithVirtualScreen(false);
    for (auto& surface : surfaces_) {
        if (surface.hwnd) ShowWindow(surface.hwnd, SW_SHOWNA);
    }
    SetTimer(timerHwnd_, kTimerId, 16, nullptr);
    EnsureTopmostZOrder(true);
}

void OverlayHostWindow::StopFrameLoop() {
    if (!ticking_) return;
    ticking_ = false;
    if (timerHwnd_) {
        KillTimer(timerHwnd_, kTimerId);
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
