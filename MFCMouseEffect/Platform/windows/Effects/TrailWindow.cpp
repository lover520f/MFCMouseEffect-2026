#include "pch.h"
#include "TrailWindow.h"
#include "MouseFx/Core/System/CursorPositionProvider.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"
#include <algorithm>

#pragma comment(lib, "gdiplus.lib")
#include <cmath>

namespace mousefx {
namespace {

constexpr uint64_t kTrailSyntheticFollowWindowMs = 48;

}


static const uint64_t kTopmostReassertIntervalMs = 2500;
static TrailWindow* g_foregroundHookOwner = nullptr;

TrailWindow::TrailWindow() = default;

TrailWindow::~TrailWindow() {
    Shutdown();
}

void TrailWindow::Shutdown() {
    UnregisterForegroundHook();
    if (hwnd_) {
        KillTimer(hwnd_, kTimerId);
        frameTimerArmed_ = false;
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
    UpdateFrameTimerForPoint(nullptr, true);
    ShowWindow(hwnd_, SW_SHOWNA);
    RegisterForegroundHook();
    EnsureTopmostZOrder(true);
    
    return true;
}

void TrailWindow::AddPoint(const TrailPoint& point) {
    latestPoint_ = point;
    hasLatestPoint_ = true;
    lastInputPointTickMs_ = point.addedTime;
    UpdateFrameTimerForPoint(&point.pt, false);
}

void TrailWindow::Clear() {
    points_.clear();
    hasLastSamplePt_ = false;
    lastInputPointTickMs_ = 0;
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
        UnregisterForegroundHook();
        KillTimer(hwnd_, kTimerId);
        frameTimerArmed_ = false;
        break;
    case kMsgEnsureTopmost:
        EnsureTopmostZOrder(true);
        return 0;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void TrailWindow::OnTick() {
    UpdateFrameTimerForPoint(nullptr, false);
    uint64_t now = NowMs();
    SampleCursorPoint(now);
    
    // Remove old points
    while (!points_.empty()) {
        const int pointDurationMs = trail_point_style::ResolveDurationMs(points_.front(), durationMs_);
        if (now - points_.front().addedTime > static_cast<uint64_t>(pointDurationMs)) {
            points_.pop_front();
        } else {
            break;
        }
    }

    // Always render (or optimize to only render if dirty? For smooth fadeout we need to render always)
    EnsureTopmostZOrder(false);
    Render();
}

void TrailWindow::UpdateFrameTimerForPoint(const ScreenPoint* hintPoint, bool force) {
    if (!hwnd_) {
        return;
    }
    int desiredIntervalMs = 16;
    if (hintPoint) {
        desiredIntervalMs = win32_overlay_timer_support::ResolveTimerIntervalMsForScreenPoint(
            hintPoint->x,
            hintPoint->y);
    } else {
        desiredIntervalMs = win32_overlay_timer_support::ResolveTimerIntervalMsForCursor();
    }
    desiredIntervalMs = std::clamp(desiredIntervalMs, 4, 1000);
    const UINT desiredTimerMs = static_cast<UINT>(desiredIntervalMs);
    if (!force && frameTimerArmed_ && frameTimerIntervalMs_ == desiredTimerMs) {
        return;
    }
    SetTimer(hwnd_, kTimerId, desiredTimerMs, nullptr);
    frameTimerIntervalMs_ = desiredTimerMs;
    frameTimerArmed_ = true;
}

void TrailWindow::SampleCursorPoint(uint64_t nowMs) {
    TrailPoint incoming{};
    ScreenPoint pt{};
    bool havePoint = false;
    if (hasLatestPoint_) {
        incoming = latestPoint_;
        pt = incoming.pt;
        hasLatestPoint_ = false;
        havePoint = true;
    } else {
        if (lastInputPointTickMs_ != 0 &&
            nowMs >= lastInputPointTickMs_ &&
            (nowMs - lastInputPointTickMs_) <= kTrailSyntheticFollowWindowMs &&
            TryGetCursorScreenPoint(&pt)) {
            havePoint = true;
        }
    }
    if (!havePoint) return;

    if (hasLastSamplePt_ && pt.x == lastSamplePt_.x && pt.y == lastSamplePt_.y) {
        return;
    }

    TrailPoint tp = incoming;
    tp.pt.x = pt.x;
    tp.pt.y = pt.y;
    tp.addedTime = nowMs;
    if (tp.durationMs <= 0) {
        tp.durationMs = durationMs_;
    }
    if (tp.lineWidthPx <= 0.0) {
        tp.lineWidthPx = 4.0;
    }
    if (tp.strokeArgb == 0) {
        tp.strokeArgb = (0xFFu << 24) |
            (static_cast<uint32_t>(color_.GetR()) << 16) |
            (static_cast<uint32_t>(color_.GetG()) << 8) |
            static_cast<uint32_t>(color_.GetB());
    }
    if (tp.fillArgb == 0) {
        tp.fillArgb = (0x66u << 24) | (tp.strokeArgb & 0x00FFFFFFu);
    }
    points_.push_back(tp);
    if (points_.size() > (size_t)maxPoints_) {
        points_.pop_front();
    }
    lastSamplePt_ = pt;
    hasLastSamplePt_ = true;
}

void CALLBACK TrailWindow::ForegroundEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    TrailWindow* self = g_foregroundHookOwner;
    if (!self || !self->hwnd_) return;
    if (!IsWindow(self->hwnd_)) return;
    if (hwnd == self->hwnd_) return;
    PostMessageW(self->hwnd_, kMsgEnsureTopmost, 0, 0);
}

void TrailWindow::RegisterForegroundHook() {
    if (foregroundHook_) return;
    foregroundHook_ = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND,
        nullptr,
        &TrailWindow::ForegroundEventProc,
        0,
        0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    if (foregroundHook_) {
        g_foregroundHookOwner = this;
    }
}

void TrailWindow::UnregisterForegroundHook() {
    if (foregroundHook_) {
        UnhookWinEvent(foregroundHook_);
        foregroundHook_ = nullptr;
    }
    if (g_foregroundHookOwner == this) {
        g_foregroundHookOwner = nullptr;
    }
}

void TrailWindow::EnsureTopmostZOrder(bool force) {
    if (!hwnd_) return;
    const uint64_t now = NowMs();
    if (!force && (now - lastTopmostEnsureMs_ < kTopmostReassertIntervalMs)) return;
    lastTopmostEnsureMs_ = now;
    SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
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
            
            const int pointDurationMs = trail_point_style::ResolveDurationMs(p1, durationMs_);
            uint64_t age = now - p1.addedTime;
            float life = 1.0f - (static_cast<float>(age) / static_cast<float>(pointDurationMs));
            if (life < 0) life = 0;
            
            int alpha = (int)(life * 255);
            Gdiplus::Color c = trail_point_style::ResolveStrokeColor(p1, color_, alpha);
             if (isChromatic_) {
                 float hue = std::fmod((float)now * 0.1f + i * 10.0f, 360.0f);
                 c = trail_color::HslToRgbColor(hue, 0.8f, 0.6f, (BYTE)alpha);
            }
            const float width = trail_point_style::ResolveLineWidthPx(p1, 4.0f);
            Gdiplus::Pen p(c, width); 
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
