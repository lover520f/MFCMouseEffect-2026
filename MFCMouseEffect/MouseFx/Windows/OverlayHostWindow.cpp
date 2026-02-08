#include "pch.h"

#include "OverlayHostWindow.h"
#include "MouseFx/Core/OverlayCoordSpace.h"

#include <algorithm>

namespace mousefx {

static uint64_t NowMs() {
    return GetTickCount64();
}

static const uint64_t kTopmostReassertIntervalMs = 2500;
static OverlayHostWindow* g_overlayForegroundHookOwner = nullptr;

OverlayHostWindow::OverlayHostWindow() = default;

OverlayHostWindow::~OverlayHostWindow() {
    Shutdown();
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
    if (hwnd_) return true;
    if (!EnsureClassRegistered()) return false;

    const int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    const DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    hwnd_ = CreateWindowExW(
        ex,
        ClassName(),
        L"",
        WS_POPUP,
        x, y, w, h,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this);
    if (!hwnd_) return false;
    SetOverlayWindowHandle(hwnd_);

    EnsureSurface(w, h);
    ShowWindow(hwnd_, SW_HIDE);
    RegisterForegroundHook();
    SyncBoundsWithVirtualScreen(true);
    EnsureTopmostZOrder(true);
    return true;
}

void OverlayHostWindow::Shutdown() {
    StopFrameLoop();
    UnregisterForegroundHook();
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    DestroySurface();
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

LRESULT CALLBACK OverlayHostWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OverlayHostWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<OverlayHostWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<OverlayHostWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) return self->OnMessage(msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT OverlayHostWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_TIMER:
        if (wParam == kTimerId) {
            OnTick();
            return 0;
        }
        break;
    case kMsgEnsureTopmost:
        EnsureTopmostZOrder(true);
        return 0;
    case WM_DESTROY:
        StopFrameLoop();
        UnregisterForegroundHook();
        ClearOverlayWindowHandle();
        ClearOverlayOriginOverride();
        break;
    case WM_DISPLAYCHANGE:
    case WM_DPICHANGED:
        SyncBoundsWithVirtualScreen(true);
        return 0;
    default:
        break;
    }

    return DefWindowProcW(hwnd_, msg, wParam, lParam);
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
    if (!hwnd_ || !memDc_ || !bits_) return;

    RECT rc{};
    if (GetWindowRect(hwnd_, &rc)) {
        SetOverlayOriginOverride(rc.left, rc.top);
    }

    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);

    Gdiplus::Bitmap bmp(width_, height_, width_ * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics graphics(&bmp);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    for (auto& layer : layers_) {
        if (layer && layer->IsAlive()) {
            layer->Render(graphics);
        }
    }

    UpdateLayered();
}

void OverlayHostWindow::UpdateLayered() {
    POINT ptSrc{0, 0};
    SIZE sizeWnd{width_, height_};
    POINT ptDst{virtualX_, virtualY_};
    SetOverlayOriginOverride(ptDst.x, ptDst.y);
    BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sizeWnd, memDc_, &ptSrc, 0, &bf, ULW_ALPHA);
}

void OverlayHostWindow::EnsureSurface(int w, int h) {
    if (width_ == w && height_ == h && memDc_) return;
    DestroySurface();
    width_ = w;
    height_ = h;

    HDC screen = GetDC(nullptr);
    memDc_ = CreateCompatibleDC(screen);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    dib_ = CreateDIBSection(screen, &bmi, DIB_RGB_COLORS, &bits_, nullptr, 0);
    if (dib_) SelectObject(memDc_, dib_);
    ReleaseDC(nullptr, screen);
}

void OverlayHostWindow::DestroySurface() {
    if (dib_) DeleteObject(dib_);
    if (memDc_) DeleteDC(memDc_);
    dib_ = nullptr;
    memDc_ = nullptr;
    bits_ = nullptr;
    width_ = 0;
    height_ = 0;
}

void OverlayHostWindow::SyncBoundsWithVirtualScreen(bool forceMove) {
    if (!hwnd_) return;

    const int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (w <= 0 || h <= 0) return;

    const bool boundsChanged = (x != virtualX_) || (y != virtualY_) || (w != virtualW_) || (h != virtualH_);
    if (forceMove || boundsChanged) {
        SetWindowPos(hwnd_, nullptr, x, y, w, h, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        virtualX_ = x;
        virtualY_ = y;
        virtualW_ = w;
        virtualH_ = h;
    }

    EnsureSurface(w, h);
    SetOverlayOriginOverride(virtualX_, virtualY_);
}

void OverlayHostWindow::StartFrameLoop() {
    if (!hwnd_) return;
    if (ticking_) return;
    ticking_ = true;
    SyncBoundsWithVirtualScreen(false);
    ShowWindow(hwnd_, SW_SHOWNA);
    SetTimer(hwnd_, kTimerId, 16, nullptr);
    EnsureTopmostZOrder(true);
}

void OverlayHostWindow::StopFrameLoop() {
    if (!hwnd_) return;
    if (!ticking_) return;
    ticking_ = false;
    KillTimer(hwnd_, kTimerId);
    ShowWindow(hwnd_, SW_HIDE);
}

void OverlayHostWindow::EnsureTopmostZOrder(bool force) {
    if (!hwnd_) return;
    const uint64_t now = NowMs();
    if (!force && (now - lastTopmostEnsureMs_ < kTopmostReassertIntervalMs)) return;
    lastTopmostEnsureMs_ = now;
    SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void CALLBACK OverlayHostWindow::ForegroundEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    OverlayHostWindow* self = g_overlayForegroundHookOwner;
    if (!self || !self->hwnd_) return;
    if (!IsWindow(self->hwnd_)) return;
    if (hwnd == self->hwnd_) return;
    PostMessageW(self->hwnd_, kMsgEnsureTopmost, 0, 0);
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
