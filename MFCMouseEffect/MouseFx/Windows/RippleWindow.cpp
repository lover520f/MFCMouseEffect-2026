// RippleWindow.cpp

#include "pch.h"

#include "RippleWindow.h"
#include "MouseFx/Renderers/Click/RippleRenderer.h" // Default strategy

#include <algorithm>
#include <cmath>

#pragma comment(lib, "gdiplus.lib")

namespace mousefx {

static uint64_t NowMs() {
    return GetTickCount64();
}

RippleWindow::~RippleWindow() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    DestroySurface();
}

const wchar_t* RippleWindow::ClassName() {
    return L"MouseFxRippleWindow";
}

bool RippleWindow::EnsureClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &RippleWindow::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = ClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

bool RippleWindow::Create() {
    if (hwnd_) return true;
    if (!EnsureClassRegistered()) return false;

    DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    hwnd_ = CreateWindowExW(
        ex,
        ClassName(),
        L"",
        WS_POPUP,
        0, 0, 0, 0,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this
    );
    if (!hwnd_) return false;

    ShowWindow(hwnd_, SW_HIDE);
    return true;
}

// Convenience: uses default RippleRenderer
void RippleWindow::StartAt(const ClickEvent& ev, std::unique_ptr<IRippleRenderer> renderer) {
    if (!renderer) renderer = std::make_unique<RippleRenderer>();
    StartContinuous(ev, std::move(renderer));
    continuous_ = false; // Regular one-shot
}

void RippleWindow::StartContinuous(const ClickEvent& ev, std::unique_ptr<IRippleRenderer> renderer) {
    if (!hwnd_ && !Create()) return;

    renderer_ = std::move(renderer);
    if (!renderer_) return; // Should not happen given check above, but for safety

    current_ = ev;
    continuous_ = true;
    loop_ = true;

    // Default style override for simple ripple if none provided
    style_ = RippleStyle{};
    switch (ev.button) {
    case MouseButton::Left:
        style_.fill = {0x594FC3F7};
        style_.stroke = {0xFF0288D1};
        style_.glow = {0x660288D1};
        break;
    case MouseButton::Right:
        style_.fill = {0x50FFB74D};
        style_.stroke = {0xFFFF6F00};
        style_.glow = {0x55FF6F00};
        break;
    case MouseButton::Middle:
        style_.fill = {0x5033D17A};
        style_.stroke = {0xFF0B8043};
        style_.glow = {0x550B8043};
        break;
    }

    // Init renderer
    renderer_->Start(style_);

    EnsureSurface(style_.windowSize);

    const int left = static_cast<int>(ev.pt.x - (style_.windowSize / 2));
    const int top = static_cast<int>(ev.pt.y - (style_.windowSize / 2));

    SetWindowPos(hwnd_, HWND_TOPMOST, left, top, style_.windowSize, style_.windowSize,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    startTick_ = NowMs();
    active_ = true;

    RenderFrame(0.0f, 0);
    SetTimer(hwnd_, kTimerId, 16, nullptr);
}

void RippleWindow::StartAt(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    StartContinuous(ev, style, std::move(renderer), params);
    continuous_ = false;
}

void RippleWindow::StartContinuous(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    if (!hwnd_ && !Create()) return;

    renderer_ = std::move(renderer);
    if (!renderer_) return;

    current_ = ev;
    continuous_ = true;
    loop_ = params.loop;
    style_ = style;
    render_ = params;

    // Init
    renderer_->SetParams(render_);
    renderer_->Start(style_);

    EnsureSurface(style_.windowSize);

    int left = static_cast<int>(ev.pt.x - (style_.windowSize / 2));
    int top = static_cast<int>(ev.pt.y - (style_.windowSize / 2));

    SetWindowPos(hwnd_, HWND_TOPMOST, left, top, style_.windowSize, style_.windowSize,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    startTick_ = NowMs();
    active_ = true;

    RenderFrame(0.0f, 0);
    SetTimer(hwnd_, kTimerId, 16, nullptr);
}

void RippleWindow::UpdatePosition(const POINT& pt) {
    if (!active_ || !hwnd_) return;
    
    // Recalculate position based on new center pt
    const int left = static_cast<int>(pt.x - (style_.windowSize / 2));
    const int top = static_cast<int>(pt.y - (style_.windowSize / 2));
    
    // Move window (SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE)
    SetWindowPos(hwnd_, nullptr, left, top, 0, 0,
        SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void RippleWindow::Stop() {
    active_ = false;
    if (hwnd_) {
        KillTimer(hwnd_, kTimerId);
        ShowWindow(hwnd_, SW_HIDE);
    }
}

LRESULT CALLBACK RippleWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    RippleWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<RippleWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<RippleWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->OnMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT RippleWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_ERASEBKGND:
        return 1;
    case WM_TIMER:
        if (wParam == kTimerId) {
            OnTick();
            return 0;
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd_, kTimerId);
        active_ = false;
        break;
    default:
        break;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void RippleWindow::OnTick() {
    if (!active_) {
        Stop();
        return;
    }

    const uint64_t elapsed = NowMs() - startTick_;
    float t = (style_.durationMs == 0) ? 1.0f : (float)elapsed / (float)style_.durationMs;

    if (continuous_) {
        if (loop_) {
            if (t > 1.0f) {
                startTick_ = NowMs();
                t = 0.0f;
                // Notify strategy of loop reset if needed?
                // For now, renderer handles physics based on elapsedMs usually, 
                // but if it relies on 't', it just resets.
                // If it relies on elapsedMs state (like particles), 
                // we might want Re-Start? 
                // For Lightning, "Start" reseeds particles.
                // If we loop, we might want to re-seed?
                // Let's call data reset if t loops.
                if (renderer_) renderer_->Start(style_);
            }
        } else {
            if (t > 1.0f) {
                t = 1.0f;
            }
        }
    } else {
        if (t >= 1.0f) {
            Stop();
            return;
        }
    }

    RenderFrame(t, elapsed);
}

void RippleWindow::RenderFrame(float t, uint64_t elapsedMs) {
    if (!hwnd_ || !memDc_ || !dib_ || !bits_ || sizePx_ <= 0 || !renderer_) return;

    // Clear to fully transparent (premultiplied alpha).
    const int stride = sizePx_ * 4;
    ZeroMemory(bits_, (size_t)stride * (size_t)sizePx_);

    Gdiplus::Bitmap bmp(sizePx_, sizePx_, stride, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    // Delegate to Strategy
    renderer_->Render(g, t, elapsedMs, sizePx_, style_);

    // Push to screen
    POINT ptSrc{ 0, 0 };
    SIZE sizeWnd{ sizePx_, sizePx_ };
    RECT r{};
    GetWindowRect(hwnd_, &r);
    POINT ptDst{ r.left, r.top };

    BLENDFUNCTION bf{};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sizeWnd, memDc_, &ptSrc, 0, &bf, ULW_ALPHA);
}

void RippleWindow::EnsureSurface(int sizePx) {
    if (sizePx < 64) sizePx = 64;
    if (sizePx > 512) sizePx = 512;
    if (sizePx_ == sizePx && memDc_ && dib_ && bits_) return;

    DestroySurface();
    sizePx_ = sizePx;

    HDC screen = GetDC(nullptr);
    memDc_ = CreateCompatibleDC(screen);

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = sizePx_;
    bmi.bmiHeader.biHeight = -sizePx_; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    dib_ = CreateDIBSection(screen, &bmi, DIB_RGB_COLORS, &bits_, nullptr, 0);
    if (dib_) {
        SelectObject(memDc_, dib_);
    }
    ReleaseDC(nullptr, screen);
}

void RippleWindow::DestroySurface() {
    bits_ = nullptr;
    if (dib_) {
        DeleteObject(dib_);
        dib_ = nullptr;
    }
    if (memDc_) {
        DeleteDC(memDc_);
        memDc_ = nullptr;
    }
    sizePx_ = 0;
}

} // namespace mousefx
