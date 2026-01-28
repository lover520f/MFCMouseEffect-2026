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

static float fnmod(float x, float y) {
    return x - y * floor(x / y);
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
    StartContinuous(ev);
    continuous_ = false; // Regular one-shot
}

void RippleWindow::StartContinuous(const ClickEvent& ev) {
    if (!hwnd_ && !Create()) return;

    current_ = ev;
    continuous_ = true;
    loop_ = true;

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
    RenderFrame(0.0f, 0);
    SetTimer(hwnd_, kTimerId, 16, nullptr);
}

void RippleWindow::StartAt(const ClickEvent& ev, const RippleStyle& style, DrawMode mode, const RenderParams& params) {
    StartContinuous(ev, style, mode, params);
    continuous_ = false;
}

void RippleWindow::StartContinuous(const ClickEvent& ev, const RippleStyle& style, DrawMode mode, const RenderParams& params) {
    if (!hwnd_ && !Create()) return;

    current_ = ev;
    continuous_ = true;
    loop_ = params.loop;
    style_ = style;
    drawMode_ = mode;
    render_ = params;

    EnsureSurface(style_.windowSize);
    
    if (drawMode_ == DrawMode::LightningSingularity) {
        InitLightningParticles();
    }

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

void RippleWindow::RenderFrame(float t, uint64_t elapsedMs) {
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
    } else if (drawMode_ == DrawMode::ScrollChevron) {
        const float intensity = Clamp01(render_.intensity);
        const float alpha = (1.0f - eased) * intensity;

        const float radius = style_.startRadius + (style_.endRadius - style_.startRadius) * eased;
        const float cx = sizePx_ / 2.0f;
        const float cy = sizePx_ / 2.0f;

        const float dir = render_.directionRad;
        const float dx = (float)cos(dir);
        const float dy = (float)sin(dir);
        const float px = -dy;
        const float py = dx;

        const Gdiplus::Color base = ToGdiPlus(style_.stroke);
        const BYTE aBase = ClampByte((int)(base.GetA() * alpha));

        for (int i = 0; i < 3; ++i) {
            const float offset = i * (radius * 0.25f);
            const float length = radius * 1.1f;
            const float halfWidth = style_.strokeWidth * (3.2f - i * 0.6f);

            const float tipX = cx + dx * (length * 0.5f - offset);
            const float tipY = cy + dy * (length * 0.5f - offset);
            const float tailX = cx - dx * (length * 0.5f + offset);
            const float tailY = cy - dy * (length * 0.5f + offset);

            const float lx = tailX + px * halfWidth;
            const float ly = tailY + py * halfWidth;
            const float rx = tailX - px * halfWidth;
            const float ry = tailY - py * halfWidth;

            const float fade = 1.0f - i * 0.18f;
            const Gdiplus::Color glow = ToGdiPlus(style_.glow);
            const BYTE a = ClampByte((int)(aBase * fade));
            const BYTE glowA = ClampByte((int)(glow.GetA() * alpha * fade));
            Gdiplus::Pen glowPen(Gdiplus::Color(glowA, glow.GetR(), glow.GetG(), glow.GetB()),
                style_.strokeWidth + 6.0f);
            glowPen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawLine(&glowPen, tipX, tipY, lx, ly);
            g.DrawLine(&glowPen, tipX, tipY, rx, ry);

            Gdiplus::Pen pen(Gdiplus::Color(a, base.GetR(), base.GetG(), base.GetB()),
                style_.strokeWidth + 1.0f);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawLine(&pen, tipX, tipY, lx, ly);
            g.DrawLine(&pen, tipX, tipY, rx, ry);
        }
    } else if (drawMode_ == DrawMode::ChargeRing) {
        const float cx = sizePx_ / 2.0f;
        const float cy = sizePx_ / 2.0f;
        const float radius = style_.endRadius;
        const float progress = Clamp01(t);
        const float pulse = 0.5f + 0.5f * (float)sin((double)elapsedMs * 0.0045);
        const float alpha = 0.55f + 0.45f * pulse;

        const Gdiplus::Color stroke = ToGdiPlus(style_.stroke);
        const BYTE aStroke = ClampByte((int)(stroke.GetA() * alpha));
        const Gdiplus::Color glow = ToGdiPlus(style_.glow);
        const BYTE aGlow = ClampByte((int)(glow.GetA() * alpha * 0.6f));

        // Background ring
        Gdiplus::Pen bgPen(Gdiplus::Color(ClampByte((int)(aStroke * 0.25f)),
            stroke.GetR(), stroke.GetG(), stroke.GetB()), style_.strokeWidth);
        bgPen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawEllipse(&bgPen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        // Glow arc
        Gdiplus::Pen glowPen(Gdiplus::Color(aGlow, glow.GetR(), glow.GetG(), glow.GetB()),
            style_.strokeWidth + 8.0f);
        glowPen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawArc(&glowPen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, -90.0f, progress * 360.0f);

        // Progress arc
        Gdiplus::Pen pen(Gdiplus::Color(aStroke, stroke.GetR(), stroke.GetG(), stroke.GetB()),
            style_.strokeWidth + 1.0f);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawArc(&pen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, -90.0f, progress * 360.0f);

        // Accent dot at head
        const float angle = (-90.0f + progress * 360.0f) * 3.1415926f / 180.0f;
        const float dotX = cx + (float)cos(angle) * radius;
        const float dotY = cy + (float)sin(angle) * radius;
        const float dotR = style_.strokeWidth * 2.2f;
        Gdiplus::SolidBrush dotBrush(Gdiplus::Color(aStroke, stroke.GetR(), stroke.GetG(), stroke.GetB()));
        g.FillEllipse(&dotBrush, dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
    } else if (drawMode_ == DrawMode::HoverCrosshair) {
        const float cx = sizePx_ / 2.0f;
        const float cy = sizePx_ / 2.0f;
        const float radius = style_.endRadius * 0.9f;
        const float phase = (float)fmod((double)elapsedMs / (double)style_.durationMs, 1.0);
        const float pulse = 0.4f + 0.6f * (float)sin(phase * 2.0f * 3.1415926f);
        const float alpha = Clamp01(pulse);

        const Gdiplus::Color stroke = ToGdiPlus(style_.stroke);
        const BYTE aStroke = ClampByte((int)(stroke.GetA() * alpha));
        const Gdiplus::Color glow = ToGdiPlus(style_.glow);
        const BYTE aGlow = ClampByte((int)(glow.GetA() * alpha * 0.6f));

        Gdiplus::Pen glowPen(Gdiplus::Color(aGlow, glow.GetR(), glow.GetG(), glow.GetB()),
            style_.strokeWidth + 6.0f);
        glowPen.SetLineJoin(Gdiplus::LineJoinRound);

        Gdiplus::Pen pen(Gdiplus::Color(aStroke, stroke.GetR(), stroke.GetG(), stroke.GetB()),
            style_.strokeWidth + 0.5f);
        pen.SetLineJoin(Gdiplus::LineJoinRound);

        const float tick = radius * 0.22f;
        g.DrawLine(&glowPen, cx - radius, cy, cx - radius + tick, cy);
        g.DrawLine(&glowPen, cx + radius - tick, cy, cx + radius, cy);
        g.DrawLine(&glowPen, cx, cy - radius, cx, cy - radius + tick);
        g.DrawLine(&glowPen, cx, cy + radius - tick, cx, cy + radius);

        g.DrawLine(&pen, cx - radius, cy, cx - radius + tick, cy);
        g.DrawLine(&pen, cx + radius - tick, cy, cx + radius, cy);
        g.DrawLine(&pen, cx, cy - radius, cx, cy - radius + tick);
        g.DrawLine(&pen, cx, cy + radius - tick, cx, cy + radius);

        // Orbiting dot
        const float dotAngle = phase * 2.0f * 3.1415926f;
        const float dotR = style_.strokeWidth * 1.6f;
        const float dotX = cx + (float)cos(dotAngle) * (radius * 0.6f);
        const float dotY = cy + (float)sin(dotAngle) * (radius * 0.6f);
        Gdiplus::SolidBrush dotBrush(Gdiplus::Color(aStroke, stroke.GetR(), stroke.GetG(), stroke.GetB()));
        g.FillEllipse(&dotBrush, dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
    } else if (drawMode_ == DrawMode::LightningSingularity) {
        const float cx = sizePx_ / 2.0f;
        const float cy = sizePx_ / 2.0f;
        const float radius = style_.endRadius;
        const float maxEnergy = 1.0f;
        
        // Central Core
        // Grows as t increases to simulate energy buildup
        float coreRadius = style_.strokeWidth * (2.0f + t * 4.0f); // 2x -> 6x
        // Pulse the core
        float pulse = 0.5f + 0.5f * (float)sin((double)elapsedMs * 0.05); // Fast pulse
        float coreAlpha = 0.8f + 0.2f * pulse;
        
        Gdiplus::Color glow = ToGdiPlus(style_.glow);
        Gdiplus::Color coreColor = ToGdiPlus(style_.stroke);
        
        // Core Glow
        {
             Gdiplus::GraphicsPath path;
             path.AddEllipse(cx - coreRadius * 2.5f, cy - coreRadius * 2.5f, coreRadius * 5.0f, coreRadius * 5.0f);
             Gdiplus::PathGradientBrush pgb(&path);
             pgb.SetCenterColor(Gdiplus::Color(ClampByte((int)(glow.GetA() * 0.6f)), glow.GetR(), glow.GetG(), glow.GetB()));
             Gdiplus::Color surround[1] = { Gdiplus::Color(0, glow.GetR(), glow.GetG(), glow.GetB()) };
             int count = 1;
             pgb.SetSurroundColors(surround, &count);
             pgb.SetCenterPoint(Gdiplus::PointF(cx, cy));
             g.FillPath(&pgb, &path);
        }

        // Draw Core
        Gdiplus::SolidBrush coreBrush(Gdiplus::Color(ClampByte((int)(coreColor.GetA() * coreAlpha)), coreColor.GetR(), coreColor.GetG(), coreColor.GetB()));
        g.FillEllipse(&coreBrush, cx - coreRadius, cy - coreRadius, coreRadius * 2.0f, coreRadius * 2.0f);
        
        // Draw Particles / Lightning streaks
        srand((unsigned int)startTick_); // Deterministic for consistent flickering if needed, but here we update per frame maybe?
        
        Gdiplus::Pen boltPen(Gdiplus::Color(ClampByte((int)(coreColor.GetA() * 0.7f)), coreColor.GetR(), coreColor.GetG(), coreColor.GetB()), style_.strokeWidth * 0.5f);
        
        for (auto& p : lightningParticles_) {
            // Particle moves from outside in
            // Distance depends on t. t=0 -> far, t=1 -> center
            // Let's cycle based on speed
            
            float linearProgress = (fnmod((float)elapsedMs * 0.001f * p.speed + p.phase, 1.0f)); 
            // We want them to fly IN. 1.0 -> 0.0
            float distFactor = 1.0f - linearProgress;
            
            float currentDist = radius * 0.3f + (radius * 0.8f) * distFactor + p.distOffset * 10.0f;
            if (currentDist > radius) currentDist = radius; // Clamp
            
            float angle = p.angle;
            
            // Draw a little 'streak' or 'bolt'
            float length = 15.0f * (1.0f + p.speed);
            
            float x1 = cx + (float)cos(angle) * currentDist;
            float y1 = cy + (float)sin(angle) * currentDist;
            float x2 = cx + (float)cos(angle) * (currentDist + length);
            float y2 = cy + (float)sin(angle) * (currentDist + length);
            
            // Random jitter for lightning feel
            float jitter = 2.0f * ((float)(rand() % 100) / 100.0f - 0.5f);
            
            g.DrawLine(&boltPen, x1 + jitter, y1 + jitter, x2, y2);
        }
    } else if (drawMode_ == DrawMode::HexForceField) {
        const float cx = sizePx_ / 2.0f;
        const float cy = sizePx_ / 2.0f;
        const float radius = style_.endRadius; // Base radius
        const float progress = Clamp01(t); 
        
        // Pulse alpha
        const float pulse = 0.5f + 0.5f * (float)sin((double)elapsedMs * 0.003);
        const float alphaBase = 0.6f + 0.4f * pulse;
        
        Gdiplus::Color stroke = ToGdiPlus(style_.stroke);
        Gdiplus::Color glow = ToGdiPlus(style_.glow);
        Gdiplus::Color fill = ToGdiPlus(style_.fill); 
        
        // Helper to draw a hex
        auto DrawHex = [&](float r, float rotRad, float width, float aFactor, bool fillHex) {
            Gdiplus::PointF pts[6];
            for (int i = 0; i < 6; ++i) {
                float angle = rotRad + (float)i * (3.14159f / 3.0f);
                pts[i].X = cx + r * (float)cos(angle);
                pts[i].Y = cy + r * (float)sin(angle);
            }
            
            BYTE a = ClampByte((int)(stroke.GetA() * alphaBase * aFactor));
            Gdiplus::Color c(a, stroke.GetR(), stroke.GetG(), stroke.GetB());
            
            if (fillHex) {
                 BYTE fa = ClampByte((int)(fill.GetA() * alphaBase * aFactor * 0.3f));
                 Gdiplus::SolidBrush b(Gdiplus::Color(fa, fill.GetR(), fill.GetG(), fill.GetB()));
                 g.FillPolygon(&b, pts, 6);
            }

            Gdiplus::Pen p(c, width);
            p.SetLineJoin(Gdiplus::LineJoinMiter); // Sharp corners for hex
            g.DrawPolygon(&p, pts, 6);
            
            // Draw glow vertices
            BYTE ga = ClampByte((int)(glow.GetA() * alphaBase * aFactor));
            Gdiplus::SolidBrush gb(Gdiplus::Color(ga, glow.GetR(), glow.GetG(), glow.GetB()));
            float dotR = width * 1.5f;
            for(int i=0; i<6; ++i) {
                g.FillEllipse(&gb, pts[i].X - dotR, pts[i].Y - dotR, dotR*2, dotR*2);
            }
        };
        
        float timeSec = (float)elapsedMs / 1000.0f;
        
        // 1. Inner Hex: Fast, Clockwise
        // Grows quickly from 0 to target size
        float r1 = radius * 0.4f * progress;
        if (r1 > 1.0f) DrawHex(r1, timeSec * 2.0f, style_.strokeWidth, 1.0f, true);
        
        // 2. Middle Hex: Slow, Counter-Clockwise
        float r2 = radius * 0.7f * progress;
        if (r2 > 1.0f) DrawHex(r2, -timeSec * 0.5f, style_.strokeWidth * 0.8f, 0.7f, false);
        
        // 3. Outer Hex: Medium, Clockwise
        float r3 = radius * 1.0f * progress;
        if (r3 > 1.0f) DrawHex(r3, timeSec * 1.0f + 0.5f, style_.strokeWidth * 0.5f, 0.4f, false);
        
    } else {
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

void RippleWindow::InitLightningParticles() {
    lightningParticles_.clear();
    // Create ~20 particles
    for (int i = 0; i < 24; ++i) {
        LightningParticle p;
        p.angle = ((float)(rand() % 360) / 180.0f) * 3.14159f;
        p.speed = 1.0f + ((float)(rand() % 100) / 50.0f); // 1.0 - 3.0
        p.phase = ((float)(rand() % 100) / 100.0f);
        p.distOffset = ((float)(rand() % 100) / 100.0f) * 5.0f;
        p.active = true;
        lightningParticles_.push_back(p);
    }
}

} // namespace mousefx
