#include "pch.h"
#include "ParticleTrailWindow.h"
#include "MouseFx/Core/Effects/TrailStyleCompute.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"
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

static Gdiplus::Color ArgbWithOpacity(uint32_t argb, float opacityScale) {
    const int baseAlpha = static_cast<int>((argb >> 24) & 0xFFu);
    const int alpha = std::clamp(
        static_cast<int>(std::lround(static_cast<float>(baseAlpha) * opacityScale)),
        0,
        255);
    return Gdiplus::Color(
        static_cast<BYTE>(alpha),
        static_cast<BYTE>((argb >> 16) & 0xFFu),
        static_cast<BYTE>((argb >> 8) & 0xFFu),
        static_cast<BYTE>(argb & 0xFFu));
}

ParticleTrailWindow::ParticleTrailWindow() = default;

ParticleTrailWindow::~ParticleTrailWindow() {
    Shutdown();
}

void ParticleTrailWindow::Shutdown() {
    UnregisterForegroundHook();
    if (hwnd_) {
        KillTimer(hwnd_, kTimerId);
        frameTimerArmed_ = false;
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
    rngState_ = static_cast<uint32_t>(lastTick_ & 0xFFFFFFFFu);
    if (rngState_ == 0u) {
        rngState_ = 0x7F4A7C15u;
    }
    UpdateFrameTimerForPoint(nullptr, true);
    ShowWindow(hwnd_, SW_SHOWNA);
    RegisterForegroundHook();
    EnsureTopmostZOrder(true);
    
    return true;
}

void ParticleTrailWindow::AddCommand(const TrailEffectRenderCommand& command) {
    if (command.normalizedType == "none") {
        Clear();
        return;
    }
    if (!command.emit || command.normalizedType != "particle") {
        return;
    }

    UpdateFrameTimerForPoint(&command.overlayPoint, false);

    const int emitCount = trail_style_compute::ComputeParticleEmitCount(std::max(1.0, command.speedPx));
    if (emitCount <= 0) {
        return;
    }
    Emit(command, emitCount);
}

void ParticleTrailWindow::Emit(const TrailEffectRenderCommand& command, int count) {
    if (!hwnd_ && !Create()) return;

    int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

    globalHue_ = std::fmod(globalHue_ + 5.0f, 360.0f);
    const float intensityScale = std::clamp(static_cast<float>(0.75 + command.intensity * 0.60), 0.60f, 1.80f);
    const float sizeScale = std::clamp(static_cast<float>(command.sizePx / 56.0), 0.45f, 2.40f);
    const float durationSec = std::clamp(static_cast<float>(command.durationSec), 0.08f, 3.0f);
    const float decayScale = std::clamp(0.22f / durationSec, 0.35f, 2.5f);
    const uint32_t baseArgb = command.strokeArgb != 0 ? command.strokeArgb : command.fillArgb;
    const bool useHue = isChromatic_;
    const float spawnOpacity = std::clamp(static_cast<float>(command.baseOpacity), 0.10f, 1.0f);

    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = static_cast<float>(command.overlayPoint.x) - static_cast<float>(x_offset);
        p.y = static_cast<float>(command.overlayPoint.y) - static_cast<float>(y_offset);
        const auto spawn = trail_style_compute::ComputeParticleSpawnMetrics(
            &rngState_,
            isChromatic_,
            globalHue_);
        const float speed = static_cast<float>(spawn.speedPxPerTick) * intensityScale;
        p.vx = std::cos(static_cast<float>(spawn.angleRad)) * speed;
        p.vy = std::sin(static_cast<float>(spawn.angleRad)) * speed;
        
        p.life = 1.0f;
        p.hue = static_cast<float>(spawn.hueDeg);
        p.size = static_cast<float>(spawn.sizePx) * sizeScale;
        p.renderRadiusPx = p.size * 0.5f;
        p.renderOpacity = spawnOpacity;
        p.decayScale = decayScale;
        p.baseArgb = baseArgb;
        p.useHue = useHue;
        
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
        KillTimer(hwnd_, kTimerId);
        frameTimerArmed_ = false;
        break;
    case kMsgEnsureTopmost:
        EnsureTopmostZOrder(true);
        return 0;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void ParticleTrailWindow::OnTick() {
    UpdateFrameTimerForPoint(nullptr, false);
    uint64_t now = NowMs();
    float dt = (now - lastTick_) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    lastTick_ = now;

    // Update particles
    for (auto it = particles_.begin(); it != particles_.end(); ) {
        const auto step = trail_style_compute::ComputeParticleStepMetrics(
            it->x,
            it->y,
            it->vx,
            it->vy,
            it->life,
            it->size,
            dt * it->decayScale);
        it->x = static_cast<float>(step.nextX);
        it->y = static_cast<float>(step.nextY);
        it->vx = static_cast<float>(step.nextVx);
        it->vy = static_cast<float>(step.nextVy);
        it->life = static_cast<float>(step.nextLife);
        it->renderRadiusPx = static_cast<float>(step.renderRadiusPx);
        it->renderOpacity = static_cast<float>(step.renderOpacity);
        if (it->life <= 0.0f) {
            it = particles_.erase(it);
        } else {
            ++it;
        }
    }

    EnsureTopmostZOrder(false);
    Render();
}

void ParticleTrailWindow::UpdateFrameTimerForPoint(const ScreenPoint* hintPoint, bool force) {
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
        BYTE alpha = static_cast<BYTE>(std::clamp<int>(
            static_cast<int>(std::lround(p.renderOpacity * 255.0f)),
            0,
            255));
        Gdiplus::Color color = p.useHue
            ? HslToRgb(p.hue, 0.8f, 0.6f, alpha)
            : ArgbWithOpacity(p.baseArgb, p.renderOpacity);
        Gdiplus::SolidBrush brush(color);
        
        float s = std::max(0.0f, p.renderRadiusPx * 2.0f);
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
