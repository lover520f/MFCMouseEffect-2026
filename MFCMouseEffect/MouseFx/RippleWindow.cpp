// RippleWindow.cpp

#include "pch.h"

#include "RippleWindow.h"

#include <algorithm>
#include <cmath>

#pragma comment(lib, "gdiplus.lib")

namespace mousefx {

static uint64_t NowMs() {
    return GetTickCount64();
}

static float Clamp01(float v) {
    return (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// A simple easing curve that feels snappy without being harsh.
static float EaseOutCubic(float t) {
    t = Clamp01(t);
    float u = 1.0f - t;
    return 1.0f - (u * u * u);
}

static int ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static BYTE ClampByte(int v) {
    return (BYTE)ClampInt(v, 0, 255);
}

static Gdiplus::Color ToGdiPlus(Argb c) {
    const BYTE a = (BYTE)((c.value >> 24) & 0xFF);
    const BYTE r = (BYTE)((c.value >> 16) & 0xFF);
    const BYTE g = (BYTE)((c.value >> 8) & 0xFF);
    const BYTE b = (BYTE)(c.value & 0xFF);
    return Gdiplus::Color(a, r, g, b);
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

void RippleWindow::StartAt(const ClickEvent& ev) {
    if (!hwnd_ && !Create()) return;

    current_ = ev;
    // Slightly different accent by button, keeps the effect expressive but consistent.
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

    EnsureSurface(style_.windowSize);

    const int left = static_cast<int>(ev.pt.x - (style_.windowSize / 2));
    const int top = static_cast<int>(ev.pt.y - (style_.windowSize / 2));

    SetWindowPos(hwnd_, HWND_TOPMOST, left, top, style_.windowSize, style_.windowSize,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    startTick_ = NowMs();
    active_ = true;

    // Render first frame immediately to avoid missing on fast clicks.
    RenderFrame(0.0f);
    SetTimer(hwnd_, kTimerId, 16, nullptr);
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
        // Stronger than WS_EX_TRANSPARENT alone: always pass hit-testing to windows below.
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
        KillTimer(hwnd_, kTimerId);
        ShowWindow(hwnd_, SW_HIDE);
        return;
    }

    const uint64_t elapsed = NowMs() - startTick_;
    const float t = (style_.durationMs == 0) ? 1.0f : (float)elapsed / (float)style_.durationMs;

    if (t >= 1.0f) {
        active_ = false;
        KillTimer(hwnd_, kTimerId);
        ShowWindow(hwnd_, SW_HIDE);
        return;
    }

    RenderFrame(t);
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

void RippleWindow::RenderFrame(float t) {
    if (!hwnd_ || !memDc_ || !dib_ || !bits_ || sizePx_ <= 0) return;

    // Clear to fully transparent (premultiplied alpha).
    const int stride = sizePx_ * 4;
    ZeroMemory(bits_, (size_t)stride * (size_t)sizePx_);

    Gdiplus::Bitmap bmp(sizePx_, sizePx_, stride, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    const float eased = EaseOutCubic(t);
    const float alpha = 1.0f - eased;

    const float radius = style_.startRadius + (style_.endRadius - style_.startRadius) * eased;
    const float cx = sizePx_ / 2.0f;
    const float cy = sizePx_ / 2.0f;

    // Soft glow (shadow) via multiple strokes; cheap but looks decent.
    const Gdiplus::Color glow = ToGdiPlus(style_.glow);
    for (int i = 0; i < 3; i++) {
        const float w = style_.strokeWidth + 10.0f + i * 4.0f;
        BYTE a = ClampByte((int)(glow.GetA() * alpha * (0.35f - i * 0.08f)));
        Gdiplus::Pen p(Gdiplus::Color(a, glow.GetR(), glow.GetG(), glow.GetB()), w);
        p.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawEllipse(&p, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    if (drawMode_ == DrawMode::IconStar) {
        // Draw Star Icon
        Gdiplus::Color fill = ToGdiPlus(style_.fill);
        // Fade out
        BYTE alphaByte = ClampByte((int)(255 * alpha));
        fill = Gdiplus::Color(alphaByte, fill.GetR(), fill.GetG(), fill.GetB());

        // Simple Star Geometry
        Gdiplus::PointF pts[10];
        const float rOuter = radius;
        const float rInner = radius * 0.4f;
        const double PI = 3.14159265358979323846;
        for (int i = 0; i < 10; i++) {
            double angle = i * PI / 5.0 - PI / 2.0; // start at top
            float r = (i % 2 == 0) ? rOuter : rInner;
            pts[i] = Gdiplus::PointF(cx + r * (float)cos(angle), cy + r * (float)sin(angle));
        }

        Gdiplus::SolidBrush brush(fill);
        g.FillPolygon(&brush, pts, 10);

        Gdiplus::Color stroke = ToGdiPlus(style_.stroke);
        stroke = Gdiplus::Color(alphaByte, stroke.GetR(), stroke.GetG(), stroke.GetB());
        Gdiplus::Pen pen(stroke, style_.strokeWidth);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawPolygon(&pen, pts, 10);
    }
    else {
        // Draw Ripple (Circle)
        // Fill
        Gdiplus::Color base = ToGdiPlus(style_.fill);
        const BYTE aCenter = ClampByte((int)(base.GetA() * alpha));
        const Gdiplus::Color center(aCenter, base.GetR(), base.GetG(), base.GetB());
        const Gdiplus::Color edge(0, base.GetR(), base.GetG(), base.GetB());

        Gdiplus::GraphicsPath path;
        path.AddEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        Gdiplus::PathGradientBrush pgb(&path);
        pgb.SetCenterColor(center);
        Gdiplus::Color surround[1] = { edge };
        int count = 1;
        pgb.SetSurroundColors(surround, &count);
        pgb.SetCenterPoint(Gdiplus::PointF(cx, cy));
        g.FillPath(&pgb, &path);

        // Stroke
        Gdiplus::Color stroke = ToGdiPlus(style_.stroke);
        stroke = Gdiplus::Color(ClampByte((int)(stroke.GetA() * alpha)), stroke.GetR(), stroke.GetG(), stroke.GetB());
        Gdiplus::Pen pen(stroke, style_.strokeWidth);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawEllipse(&pen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    // Push to screen (per-pixel alpha).
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

} // namespace mousefx
