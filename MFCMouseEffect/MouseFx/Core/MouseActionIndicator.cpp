#include "pch.h"

#include "MouseActionIndicator.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <gdiplus.h>
#include <memory>
#include <string>

namespace mousefx {

namespace {

constexpr wchar_t kWindowClassName[] = L"MouseFxMouseActionIndicatorWindow";
constexpr UINT_PTR kIndicatorTimerId = 0x4D49;

static uint64_t TickNow() {
    return GetTickCount64();
}

// RAII wrapper for GDI DC + DIBSection used by Render().
struct GdiRenderContext {
    HDC screenDc = nullptr;
    HDC memDc    = nullptr;
    HBITMAP hbmp = nullptr;
    HGDIOBJ oldBmp = nullptr;
    void* bits   = nullptr;

    GdiRenderContext() = default;

    ~GdiRenderContext() { Release(); }

    bool Init(int size) {
        screenDc = GetDC(nullptr);
        if (!screenDc) return false;
        memDc = CreateCompatibleDC(screenDc);
        if (!memDc) { Release(); return false; }

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = size;
        bmi.bmiHeader.biHeight = -size; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        hbmp = CreateDIBSection(memDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!hbmp || !bits) { Release(); return false; }

        oldBmp = SelectObject(memDc, hbmp);
        std::memset(bits, 0, static_cast<size_t>(size) * static_cast<size_t>(size) * 4u);
        return true;
    }

    void Release() {
        if (oldBmp && memDc) { SelectObject(memDc, oldBmp); oldBmp = nullptr; }
        if (memDc)    { DeleteDC(memDc);           memDc = nullptr; }
        if (screenDc) { ReleaseDC(nullptr, screenDc); screenDc = nullptr; }
        if (hbmp)     { DeleteObject(hbmp);        hbmp = nullptr; }
        bits = nullptr;
    }

    GdiRenderContext(const GdiRenderContext&) = delete;
    GdiRenderContext& operator=(const GdiRenderContext&) = delete;
};

// Build a combo-key display string from a KeyEvent.
static std::wstring BuildComboLabel(const KeyEvent& ev) {
    std::wstring result;
    const auto append = [&](const wchar_t* part) {
        if (!result.empty()) result += L'+';
        result += part;
    };

    // If the pressed key IS a modifier key, show only that modifier name.
    const bool keyIsModifier =
        ev.vkCode == VK_CONTROL || ev.vkCode == VK_LCONTROL || ev.vkCode == VK_RCONTROL ||
        ev.vkCode == VK_SHIFT   || ev.vkCode == VK_LSHIFT   || ev.vkCode == VK_RSHIFT   ||
        ev.vkCode == VK_MENU    || ev.vkCode == VK_LMENU    || ev.vkCode == VK_RMENU    ||
        ev.vkCode == VK_LWIN    || ev.vkCode == VK_RWIN;

    if (keyIsModifier) {
        return ev.text.empty() ? L"Key" : ev.text;
    }

    // Prepend held modifier names.
    if (ev.ctrl)  append(L"Ctrl");
    if (ev.shift) append(L"Shift");
    if (ev.alt)   append(L"Alt");
    if (ev.win)   append(L"Win");

    const std::wstring keyName = ev.text.empty() ? L"Key" : ev.text;
    if (!result.empty()) result += L'+';
    result += keyName;
    return result;
}

} // namespace

// ============================================================================
// Lifecycle
// ============================================================================

bool MouseActionIndicator::Initialize() {
    if (initialized_) return true;
    initialized_ = true;
    return true;
}

void MouseActionIndicator::Shutdown() {
    if (hwnd_) {
        KillTimer(hwnd_, kIndicatorTimerId);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    initialized_ = false;
    active_ = false;
}

void MouseActionIndicator::Hide() {
    active_ = false;
    eventKind_ = IndicatorEventKind::None;
    if (hwnd_) {
        KillTimer(hwnd_, kIndicatorTimerId);
        ShowWindow(hwnd_, SW_HIDE);
    }
}

void MouseActionIndicator::UpdateConfig(const MouseIndicatorConfig& cfg) {
    config_ = cfg;
    config_.positionMode = IsRelativeMode(config_.positionMode) ? "relative" : "absolute";
    config_.sizePx = ClampInt(config_.sizePx, 40, 200);
    config_.durationMs = ClampInt(config_.durationMs, 120, 2000);
    config_.offsetX = ClampInt(config_.offsetX, -2000, 2000);
    config_.offsetY = ClampInt(config_.offsetY, -2000, 2000);
    config_.absoluteX = ClampInt(config_.absoluteX, -20000, 20000);
    config_.absoluteY = ClampInt(config_.absoluteY, -20000, 20000);

    if (!config_.enabled) {
        Hide();
        return;
    }

    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, 0, 0, config_.sizePx, config_.sizePx,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    }
}

// ============================================================================
// Event handlers
// ============================================================================

void MouseActionIndicator::OnClick(const ClickEvent& ev) {
    if (!config_.enabled) return;

    const uint64_t now = TickNow();
    const uint64_t threshold = static_cast<uint64_t>(std::max<DWORD>(::GetDoubleClickTime(), 240) + 120);
    if (ev.button == lastClickButton_ && (now >= lastClickTickMs_) && (now - lastClickTickMs_ <= threshold)) {
        clickStreak_ = std::min(clickStreak_ + 1, 3);
    } else {
        clickStreak_ = 1;
    }
    lastClickButton_ = ev.button;
    lastClickTickMs_ = now;

    IndicatorEventKind kind = IndicatorEventKind::None;
    switch (ev.button) {
    case MouseButton::Left:
        kind = (clickStreak_ == 1) ? IndicatorEventKind::Left1 : (clickStreak_ == 2 ? IndicatorEventKind::Left2 : IndicatorEventKind::Left3);
        break;
    case MouseButton::Right:
        kind = (clickStreak_ == 1) ? IndicatorEventKind::Right1 : (clickStreak_ == 2 ? IndicatorEventKind::Right2 : IndicatorEventKind::Right3);
        break;
    case MouseButton::Middle:
        kind = (clickStreak_ == 1) ? IndicatorEventKind::Middle1 : (clickStreak_ == 2 ? IndicatorEventKind::Middle2 : IndicatorEventKind::Middle3);
        break;
    default:
        return;
    }

    Trigger(kind, ev.pt);
}

void MouseActionIndicator::OnScroll(const ScrollEvent& ev) {
    if (!config_.enabled) return;
    if (ev.delta == 0) return;

    // Reset click streak so scrolling doesn't carry over click counts.
    clickStreak_ = 0;

    // Accumulate streak if same direction and within timeout
    const uint64_t now = TickNow();
    const bool sameDir = (ev.delta > 0 && lastScrollDelta_ > 0) || (ev.delta < 0 && lastScrollDelta_ < 0);
    // Use system double-click time (usually 500ms) as a reasonable streak timeout
    const UINT timeout = GetDoubleClickTime();

    if (sameDir && (now >= lastScrollTickMs_) && (now - lastScrollTickMs_ < static_cast<uint64_t>(timeout))) {
        scrollStreak_++;
    } else {
        scrollStreak_ = 1;
    }
    lastScrollTickMs_ = now;
    lastScrollDelta_ = ev.delta;

    // Build label: "W+ 3" or "W-"
    std::wstring label = (ev.delta > 0) ? L"W+" : L"W-";
    if (scrollStreak_ > 1) {
        label += L" " + std::to_wstring(scrollStreak_);
    }

    const IndicatorEventKind kind = (ev.delta > 0) ? IndicatorEventKind::WheelUp : IndicatorEventKind::WheelDown;
    // Pass the label to Trigger (which sets eventLabel_, used by Render)
    Trigger(kind, ev.pt, label);
}

void MouseActionIndicator::OnKey(const KeyEvent& ev) {
    if (!config_.enabled || !config_.keyboardEnabled) return;
    std::wstring label = BuildComboLabel(ev);
    Trigger(IndicatorEventKind::KeyInput, ev.pt, std::move(label));
}

// ============================================================================
// Window management
// ============================================================================

LRESULT CALLBACK MouseActionIndicator::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* self = reinterpret_cast<MouseActionIndicator*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<MouseActionIndicator*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);
    return self->OnWndProc(hwnd, msg, wParam, lParam);
}

LRESULT MouseActionIndicator::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == kIndicatorTimerId) {
            if (!active_) {
                Hide();
                return 0;
            }
            const uint64_t now = TickNow();
            const uint64_t elapsed = (now >= eventStartMs_) ? (now - eventStartMs_) : 0;
            if (elapsed >= static_cast<uint64_t>(config_.durationMs)) {
                Hide();
                return 0;
            }
            Render();
            return 0;
        }
        break;
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool MouseActionIndicator::EnsureWindow() {
    if (hwnd_ && IsWindow(hwnd_)) return true;

    if (!windowClassRegistered_) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = &MouseActionIndicator::WndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kWindowClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        if (RegisterClassExW(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
        windowClassRegistered_ = true;
    }

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kWindowClassName,
        L"",
        WS_POPUP,
        0, 0, config_.sizePx, config_.sizePx,
        nullptr, nullptr, GetModuleHandleW(nullptr), this);

    if (!hwnd_) return false;
    return true;
}

// ============================================================================
// Trigger + Render
// ============================================================================

void MouseActionIndicator::Trigger(IndicatorEventKind kind, POINT anchorPt, std::wstring label) {
    if (!initialized_ && !Initialize()) return;
    if (!EnsureWindow()) return;

    eventKind_ = kind;
    eventStartMs_ = TickNow();
    active_ = true;
    anchorPt_ = anchorPt;
    eventLabel_ = std::move(label);

    UpdatePlacement(anchorPt);
    ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
    SetTimer(hwnd_, kIndicatorTimerId, 16, nullptr);
    Render();
}

void MouseActionIndicator::Render() {
    if (!hwnd_ || !active_) return;

    const int size = config_.sizePx;

    // RAII GDI context: screenDC + memDC + DIBSection
    GdiRenderContext ctx;
    if (!ctx.Init(size)) return;

    // Wrap the DIBSection for GDI+ drawing
    Gdiplus::Bitmap bmp(size, size, size * 4, PixelFormat32bppPARGB,
                        reinterpret_cast<BYTE*>(ctx.bits));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.Clear(Gdiplus::Color(0, 0, 0, 0));

    // Compute animation parameters
    const uint64_t now = TickNow();
    const float t = static_cast<float>((now >= eventStartMs_) ? (now - eventStartMs_) : 0)
                  / static_cast<float>(config_.durationMs);
    const IndicatorAnimParams anim = IndicatorRenderer::ComputeAnimParams(t);

    // Delegate drawing to the renderer
    if (eventKind_ == IndicatorEventKind::KeyInput) {
        renderer_.RenderKeyAction(g, size, eventLabel_, anim);
    } else {
        // Pass eventLabel_ (which might contain "W+ 3") as override
        renderer_.RenderMouseAction(g, size, eventKind_, anim, eventLabel_);
    }

    // Commit to layered window
    SIZE sz{ size, size };
    POINT src{ 0, 0 };
    RECT rc{};
    GetWindowRect(hwnd_, &rc);
    POINT dst{ rc.left, rc.top };
    BLENDFUNCTION bf{};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;
    const BOOL ok = UpdateLayeredWindow(hwnd_, ctx.screenDc, &dst, &sz,
                                        ctx.memDc, &src, 0, &bf, ULW_ALPHA);
#ifdef _DEBUG
    if (!ok) {
        wchar_t buf[128]{};
        wsprintfW(buf, L"MouseFx: MouseActionIndicator UpdateLayeredWindow failed. err=%lu\n", GetLastError());
        OutputDebugStringW(buf);
    }
#endif
}

void MouseActionIndicator::UpdatePlacement(POINT anchorPt) {
    if (!hwnd_) return;
    POINT target{};
    if (IsRelativeMode(config_.positionMode)) {
        target.x = anchorPt.x + config_.offsetX;
        target.y = anchorPt.y + config_.offsetY;
    } else {
        target.x = config_.absoluteX;
        target.y = config_.absoluteY;
    }

    const int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (vw > 0 && vh > 0) {
        const int margin = 1;
        const int minX = vx + margin;
        const int minY = vy + margin;
        const int maxX = vx + ((vw > config_.sizePx + margin) ? (vw - config_.sizePx - margin) : 0);
        const int maxY = vy + ((vh > config_.sizePx + margin) ? (vh - config_.sizePx - margin) : 0);
        target.x = ClampInt(target.x, minX, maxX);
        target.y = ClampInt(target.y, minY, maxY);
    }

    SetWindowPos(hwnd_, HWND_TOPMOST, target.x, target.y, config_.sizePx, config_.sizePx, SWP_NOACTIVATE);
}

// ============================================================================
// Utilities
// ============================================================================

int MouseActionIndicator::ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool MouseActionIndicator::IsRelativeMode(const std::string& mode) {
    return mode == "relative";
}

} // namespace mousefx
