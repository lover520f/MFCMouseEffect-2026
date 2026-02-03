#include "pch.h"
#include "TrailWindow.h"
#include "MouseFx/Utils/TrailColor.h"
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")
#include <cmath>

namespace mousefx {

static uint64_t NowMs() {
    return GetTickCount64();
}

TrailWindow::TrailWindow() = default;

TrailWindow::~TrailWindow() {
    Shutdown();
}

void TrailWindow::Shutdown() {
    if (hwnd_) {
        KillTimer(hwnd_, kTimerId);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    DestroySurface();
}

const wchar_t* TrailWindow::ClassName() {
    return L"MouseFxTrailWindow";
}

bool TrailWindow::EnsureClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &TrailWindow::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = ClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

bool TrailWindow::Create() {
    if (hwnd_) return true;
    if (!EnsureClassRegistered()) return false;

    // Full screen overlay
    // NOTE: Multimonitor support might need GetSystemMetrics(SM_CXVIRTUALSCREEN) etc.
    // For simplicity, using primary monitor now or SM_CXSCREEN.
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    hwnd_ = CreateWindowExW(
        ex,
        ClassName(),
        L"",
        WS_POPUP,
        x, y, w, h,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this
    );

    if (!hwnd_) return false;

    EnsureSurface(w, h);
    
    // Start loop
    SetTimer(hwnd_, kTimerId, 16, nullptr); // ~60fps
    ShowWindow(hwnd_, SW_SHOWNA);
    
    return true;
}

void TrailWindow::AddPoint(const POINT& pt) {
    TrailPoint tp;
    tp.pt = pt;
    tp.addedTime = NowMs();
    points_.push_back(tp);

    if (points_.size() > (size_t)maxPoints_) {
        points_.pop_front();
    }
}

void TrailWindow::Clear() {
    points_.clear();
    UpdateLayered(); // Clear screen
}

LRESULT CALLBACK TrailWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TrailWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<TrailWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<TrailWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->OnMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT TrailWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_TIMER:
        if (wParam == kTimerId) {
            OnTick();
            return 0;
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd_, kTimerId);
        break;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void TrailWindow::OnTick() {
    uint64_t now = NowMs();
    
    // Remove old points
    while (!points_.empty()) {
        if (now - points_.front().addedTime > (uint64_t)durationMs_) {
            points_.pop_front();
        } else {
            break;
        }
    }

    // Always render (or optimize to only render if dirty? For smooth fadeout we need to render always)
    Render();
}

void TrailWindow::EnsureSurface(int w, int h) {
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
    if (dib_) {
        SelectObject(memDc_, dib_);
    }
    ReleaseDC(nullptr, screen);
}

void TrailWindow::DestroySurface() {
    if (dib_) DeleteObject(dib_);
    if (memDc_) DeleteDC(memDc_);
    dib_ = nullptr;
    memDc_ = nullptr;
    bits_ = nullptr;
}

void TrailWindow::Render() {
    if (!hwnd_ || !memDc_ || !bits_) return;

    // Clear
    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);
    
    // Draw using strategy if available
    Gdiplus::Bitmap bmp(width_, height_, width_ * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    if (renderer_) {
        renderer_->Render(g, points_, width_, height_, color_, isChromatic_);
    } else {
        if (points_.size() < 2) {
             UpdateLayered();
             return;
        }
        
        uint64_t now = NowMs();
        int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        for (size_t i = 0; i < points_.size() - 1; ++i) {
            const auto& p1 = points_[i];
            const auto& p2 = points_[i+1];
            
            uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs_);
            if (life < 0) life = 0;
            
            int alpha = (int)(life * 255);
            Gdiplus::Color c(alpha, color_.GetR(), color_.GetG(), color_.GetB());
             if (isChromatic_) {
                 float hue = std::fmod((float)now * 0.1f + i * 10.0f, 360.0f);
                 c = trail_color::HslToRgbColor(hue, 0.8f, 0.6f, (BYTE)alpha);
            }
            Gdiplus::Pen p(c, 4.0f); 
            p.SetStartCap(Gdiplus::LineCapRound);
            p.SetEndCap(Gdiplus::LineCapRound);
            g.DrawLine(&p, (int)p1.pt.x - x_offset, (int)p1.pt.y - y_offset, (int)p2.pt.x - x_offset, (int)p2.pt.y - y_offset);
        }
    }
    
    UpdateLayered();
}

void TrailWindow::UpdateLayered() {
    POINT ptSrc{ 0, 0 };
    SIZE sizeWnd{ width_, height_ };
    
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    POINT ptDst{ x, y };

    BLENDFUNCTION bf{};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sizeWnd, memDc_, &ptSrc, 0, &bf, ULW_ALPHA);
}

} // namespace mousefx
