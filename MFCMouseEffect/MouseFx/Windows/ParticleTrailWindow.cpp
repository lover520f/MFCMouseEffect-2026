#include "pch.h"
#include "ParticleTrailWindow.h"
#include "MouseFx/Utils/TimeUtils.h"
#include <algorithm>
#include <cmath>

namespace mousefx {


static const uint64_t kTopmostReassertIntervalMs = 2500;
static ParticleTrailWindow* g_particleForegroundHookOwner = nullptr;

// HSV to RGB conversion
static Gdiplus::Color HslToRgb(float h, float s, float l, BYTE alpha) {
    float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;
    float r = 0, g = 0, b = 0;

    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return Gdiplus::Color(alpha, (BYTE)((r + m) * 255), (BYTE)((g + m) * 255), (BYTE)((b + m) * 255));
}

ParticleTrailWindow::ParticleTrailWindow() = default;

ParticleTrailWindow::~ParticleTrailWindow() {
    Shutdown();
}

void ParticleTrailWindow::Shutdown() {
    UnregisterForegroundHook();
    if (hwnd_) {
        KillTimer(hwnd_, kTimerId);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    DestroySurface();
}

const wchar_t* ParticleTrailWindow::ClassName() {
    return L"MouseFxParticleTrailWindow";
}

bool ParticleTrailWindow::EnsureClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &ParticleTrailWindow::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = ClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

bool ParticleTrailWindow::Create() {
    if (hwnd_) return true;
    if (!EnsureClassRegistered()) return false;

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
    
    lastTick_ = NowMs();
    SetTimer(hwnd_, kTimerId, 16, nullptr);
    ShowWindow(hwnd_, SW_SHOWNA);
    RegisterForegroundHook();
    EnsureTopmostZOrder(true);
    
    return true;
}

void ParticleTrailWindow::UpdateCursor(const POINT& pt) {
    latestCursorPt_ = pt;
    hasLatestCursorPt_ = true;
}

void ParticleTrailWindow::Emit(const POINT& pt, int count) {
    if (!hwnd_ && !Create()) return;

    int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

    static float globalHue = 0;
    globalHue = std::fmod(globalHue + 5.0f, 360.0f);

    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = (float)pt.x - x_offset;
        p.y = (float)pt.y - y_offset;
        
        // Random velocity
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = (float)(rand() % 100) / 30.0f + 0.5f;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        
        p.life = 1.0f;
        if (isChromatic_) {
             // Fully random for Chromatic
             p.hue = (float)(rand() % 360);
        } else {
             // Sequential cycling for standard (Rainbow Trace)
             p.hue = globalHue + (rand() % 40 - 20); // Variety
        }
        p.size = (float)(rand() % 40) / 10.0f + 2.0f;
        
        particles_.push_back(p);
    }
}

void ParticleTrailWindow::Clear() {
    particles_.clear();
    Render();
}

LRESULT CALLBACK ParticleTrailWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ParticleTrailWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<ParticleTrailWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<ParticleTrailWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->OnMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT ParticleTrailWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST: return HTTRANSPARENT;
    case WM_TIMER:
        if (wParam == kTimerId) {
            OnTick();
            return 0;
        }
        break;
    case WM_DESTROY:
        UnregisterForegroundHook();
        break;
    case kMsgEnsureTopmost:
        EnsureTopmostZOrder(true);
        return 0;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void ParticleTrailWindow::OnTick() {
    uint64_t now = NowMs();
    float dt = (now - lastTick_) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    lastTick_ = now;

    POINT pt{};
    bool havePt = false;
    if (hasLatestCursorPt_) {
        pt = latestCursorPt_;
        hasLatestCursorPt_ = false;
        havePt = true;
    } else if (GetCursorPos(&pt)) {
        havePt = true;
    }

    if (havePt) {
        if (!hasLastEmitCursorPt_) {
            lastEmitCursorPt_ = pt;
            hasLastEmitCursorPt_ = true;
        }
        const float dx = (float)(pt.x - lastEmitCursorPt_.x);
        const float dy = (float)(pt.y - lastEmitCursorPt_.y);
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist >= 1.0f) {
            int emitCount = (int)(dist * 0.18f) + 2;
            if (emitCount < 2) emitCount = 2;
            if (emitCount > 12) emitCount = 12;
            Emit(pt, emitCount);
            lastEmitCursorPt_ = pt;
        }
    }

    // Update particles
    for (auto it = particles_.begin(); it != particles_.end(); ) {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.05f; // Gravity
        it->life -= dt * 1.5f; // Die over ~0.6s
        
        if (it->life <= 0) {
            it = particles_.erase(it);
        } else {
            ++it;
        }
    }

    EnsureTopmostZOrder(false);
    Render();
}

void CALLBACK ParticleTrailWindow::ForegroundEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    ParticleTrailWindow* self = g_particleForegroundHookOwner;
    if (!self || !self->hwnd_) return;
    if (!IsWindow(self->hwnd_)) return;
    if (hwnd == self->hwnd_) return;
    PostMessageW(self->hwnd_, kMsgEnsureTopmost, 0, 0);
}

void ParticleTrailWindow::RegisterForegroundHook() {
    if (foregroundHook_) return;
    foregroundHook_ = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND,
        nullptr,
        &ParticleTrailWindow::ForegroundEventProc,
        0,
        0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    if (foregroundHook_) {
        g_particleForegroundHookOwner = this;
    }
}

void ParticleTrailWindow::UnregisterForegroundHook() {
    if (foregroundHook_) {
        UnhookWinEvent(foregroundHook_);
        foregroundHook_ = nullptr;
    }
    if (g_particleForegroundHookOwner == this) {
        g_particleForegroundHookOwner = nullptr;
    }
}

void ParticleTrailWindow::EnsureTopmostZOrder(bool force) {
    if (!hwnd_) return;
    const uint64_t now = NowMs();
    if (!force && (now - lastTopmostEnsureMs_ < kTopmostReassertIntervalMs)) return;
    lastTopmostEnsureMs_ = now;
    SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void ParticleTrailWindow::EnsureSurface(int w, int h) {
    if (width_ == w && height_ == h && memDc_) return;
    DestroySurface();
    width_ = w; height_ = h;
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

void ParticleTrailWindow::DestroySurface() {
    if (dib_) DeleteObject(dib_);
    if (memDc_) DeleteDC(memDc_);
    dib_ = nullptr; memDc_ = nullptr; bits_ = nullptr;
}

void ParticleTrailWindow::Render() {
    if (!hwnd_ || !memDc_ || !bits_) return;

    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);
    if (particles_.empty()) {
        UpdateLayered();
        return;
    }

    Gdiplus::Bitmap bmp(width_, height_, width_ * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    for (const auto& p : particles_) {
        BYTE alpha = (BYTE)(p.life * 255);
        Gdiplus::Color color = HslToRgb(p.hue, 0.8f, 0.6f, alpha);
        Gdiplus::SolidBrush brush(color);
        
        float s = p.size * p.life;
        g.FillEllipse(&brush, p.x - s/2, p.y - s/2, s, s);
    }
    
    UpdateLayered();
}

void ParticleTrailWindow::UpdateLayered() {
    POINT ptSrc{ 0, 0 };
    SIZE sizeWnd{ width_, height_ };
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    POINT ptDst{ x, y };
    BLENDFUNCTION bf{ AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sizeWnd, memDc_, &ptSrc, 0, &bf, ULW_ALPHA);
}

} // namespace mousefx
