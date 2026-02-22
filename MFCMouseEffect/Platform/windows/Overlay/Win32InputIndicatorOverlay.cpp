#include "pch.h"

#include "Platform/windows/Overlay/Win32InputIndicatorOverlay.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <gdiplus.h>
#include <memory>
#include <string>

#include "Platform/windows/Protocol/Win32InputTypes.h"
#include "MouseFx/Utils/MonitorUtils.h"

namespace mousefx {

namespace {

constexpr wchar_t kWindowClassName[] = L"InputIndicatorOverlayWindow";
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

    bool Init(int width, int height) {
        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        screenDc = GetDC(nullptr);
        if (!screenDc) return false;
        memDc = CreateCompatibleDC(screenDc);
        if (!memDc) { Release(); return false; }

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = safeWidth;
        bmi.bmiHeader.biHeight = -safeHeight; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        hbmp = CreateDIBSection(memDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!hbmp || !bits) { Release(); return false; }

        oldBmp = SelectObject(memDc, hbmp);
        std::memset(bits, 0, static_cast<size_t>(safeWidth) * static_cast<size_t>(safeHeight) * 4u);
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

bool Win32InputIndicatorOverlay::Initialize() {
    if (initialized_) return true;
    initialized_ = true;
    return true;
}

void Win32InputIndicatorOverlay::Shutdown() {
    DestroyClones();
    if (hwnd_) {
        KillTimer(hwnd_, kIndicatorTimerId);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    initialized_ = false;
    active_ = false;
}

void Win32InputIndicatorOverlay::Hide() {
    active_ = false;
    eventKind_ = IndicatorEventKind::None;
    if (hwnd_) {
        KillTimer(hwnd_, kIndicatorTimerId);
        ShowWindow(hwnd_, SW_HIDE);
    }
    // Hide all clone windows
    for (auto& [id, clone] : cloneWindows_) {
        if (clone && IsWindow(clone)) ShowWindow(clone, SW_HIDE);
    }
}

void Win32InputIndicatorOverlay::UpdateConfig(const InputIndicatorConfig& cfg) {
    config_ = cfg;
    config_.positionMode = IsRelativeMode(config_.positionMode) ? "relative" : "absolute";
    config_.sizePx = ClampInt(config_.sizePx, 40, 200);
    config_.durationMs = ClampInt(config_.durationMs, 120, 2000);
    config_.offsetX = ClampInt(config_.offsetX, -2000, 2000);
    config_.offsetY = ClampInt(config_.offsetY, -2000, 2000);
    config_.absoluteX = ClampInt(config_.absoluteX, -20000, 20000);
    config_.absoluteY = ClampInt(config_.absoluteY, -20000, 20000);
    UpdateRenderSize(eventKind_, eventLabel_);

    if (!config_.enabled) {
        Hide();
        return;
    }

    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, 0, 0, renderWidthPx_, renderHeightPx_,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    }
    for (auto& [id, clone] : cloneWindows_) {
        (void)id;
        if (!clone || !IsWindow(clone)) {
            continue;
        }
        SetWindowPos(clone, nullptr, 0, 0, renderWidthPx_, renderHeightPx_,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    }
}

// ============================================================================
// Event handlers
// ============================================================================

void Win32InputIndicatorOverlay::OnClick(const ClickEvent& ev) {
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

    Trigger(kind, ToNativePoint(ev.pt));
}

void Win32InputIndicatorOverlay::OnScroll(const ScrollEvent& ev) {
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
    Trigger(kind, ToNativePoint(ev.pt), label);
}

void Win32InputIndicatorOverlay::OnKey(const KeyEvent& ev) {
    if (!config_.enabled || !config_.keyboardEnabled) return;

    // Filter based on Display Mode
    if (config_.keyDisplayMode == "shortcut") {
        // Only show if modifier is held (Ctrl/Alt/Win)
        if (!ev.ctrl && !ev.alt && !ev.win) return;
    } 
    else if (config_.keyDisplayMode == "significant") {
        // ... (existing filtering logic) ...
        // Hide standard typing keys if no modifier is held
        const bool hasModifier = (ev.ctrl || ev.alt || ev.win);
        if (!hasModifier) {
            bool isTyping = false;
            // A-Z
            if (ev.vkCode >= 0x41 && ev.vkCode <= 0x5A) isTyping = true;
            // 0-9
            else if (ev.vkCode >= 0x30 && ev.vkCode <= 0x39) isTyping = true;
            // Numpad 0-9
            else if (ev.vkCode >= 0x60 && ev.vkCode <= 0x69) isTyping = true;
            // Space
            else if (ev.vkCode == VK_SPACE) isTyping = true;
            // OEM Punctuation
            else if (ev.vkCode >= 0xBA && ev.vkCode <= 0xC0) isTyping = true;
            else if (ev.vkCode >= 0xDB && ev.vkCode <= 0xDE) isTyping = true;
            else if (ev.vkCode == 0xE2) isTyping = true;
            
            if (isTyping) return;
        }
    }

    // Streak Logic
    const uint64_t now = TickNow();
    // Modifiers mask: 1=Ctrl, 2=Shift, 4=Alt, 8=Win
    uint32_t mods = (ev.ctrl ? 1 : 0) | (ev.shift ? 2 : 0) | (ev.alt ? 4 : 0) | (ev.win ? 8 : 0);
    
    // Configurable timeout? Use DoubleClickTime + delta as heuristic
    const uint64_t timeout = static_cast<uint64_t>(GetDoubleClickTime()) * 2; // e.g. 1000ms

    if (ev.vkCode == lastKeyVkCode_ && mods == lastKeyModifiers_ && 
        (now >= lastKeyTickMs_) && (now - lastKeyTickMs_ < timeout)) {
        keyStreak_++;
    } else {
        keyStreak_ = 1;
    }
    
    lastKeyVkCode_ = ev.vkCode;
    lastKeyModifiers_ = mods;
    lastKeyTickMs_ = now;

    std::wstring label = BuildComboLabel(ev);
    if (keyStreak_ > 1) {
        label += L" x" + std::to_wstring(keyStreak_);
    }

    Trigger(IndicatorEventKind::KeyInput, ToNativePoint(ev.pt), std::move(label), /*isKeyboard=*/true);
}

// ============================================================================
// Window management
// ============================================================================

LRESULT CALLBACK Win32InputIndicatorOverlay::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* self = reinterpret_cast<Win32InputIndicatorOverlay*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Win32InputIndicatorOverlay*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);
    return self->OnWndProc(hwnd, msg, wParam, lParam);
}

LRESULT Win32InputIndicatorOverlay::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
            // Render all active clone windows
            for (auto& [id, clone] : cloneWindows_) {
                if (clone && IsWindow(clone)) RenderToWindow(clone);
            }
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

bool Win32InputIndicatorOverlay::EnsureWindow() {
    if (hwnd_ && IsWindow(hwnd_)) return true;

    if (!windowClassRegistered_) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = &Win32InputIndicatorOverlay::WndProc;
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
        0, 0, renderWidthPx_, renderHeightPx_,
        nullptr, nullptr, GetModuleHandleW(nullptr), this);

    if (!hwnd_) return false;
    return true;
}

// ============================================================================
// Trigger + Render
// ============================================================================

void Win32InputIndicatorOverlay::UpdateRenderSize(IndicatorEventKind kind, const std::wstring& label) {
    renderHeightPx_ = config_.sizePx;
    renderWidthPx_ = config_.sizePx;
    if (kind != IndicatorEventKind::KeyInput) {
        return;
    }
    renderWidthPx_ = renderer_.ResolveKeyWindowWidthPx(
        config_.sizePx,
        label,
        config_.keyLabelLayoutMode);
}

void Win32InputIndicatorOverlay::Trigger(IndicatorEventKind kind, POINT anchorPt, std::wstring label, bool isKeyboard) {
    if (!initialized_ && !Initialize()) return;

    eventKind_ = kind;
    eventStartMs_ = TickNow();
    active_ = true;
    anchorPt_ = anchorPt;
    eventLabel_ = std::move(label);
    UpdateRenderSize(eventKind_, eventLabel_);

    // Custom multi-monitor mode: show on all enabled monitors
    if (IsCustomMode(isKeyboard)) {
        customModeActive_ = true;
        TriggerOnEnabledMonitors(kind, anchorPt, eventLabel_, isKeyboard);
        return;
    }

    // Single-monitor mode
    customModeActive_ = false;
    if (!EnsureWindow()) return;
    UpdatePlacement(anchorPt, isKeyboard);
    ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
    SetTimer(hwnd_, kIndicatorTimerId, 16, nullptr);
    Render();
}

void Win32InputIndicatorOverlay::Render() {
    if (!hwnd_ || !active_) return;
    RenderToWindow(hwnd_);
}

void Win32InputIndicatorOverlay::RenderToWindow(HWND targetHwnd) {
    if (!targetHwnd || !active_) return;

    const int width = std::max(1, renderWidthPx_);
    const int height = std::max(1, renderHeightPx_);
    const int stride = width * 4;

    // RAII GDI context: screenDC + memDC + DIBSection
    GdiRenderContext ctx;
    if (!ctx.Init(width, height)) return;

    // Wrap the DIBSection for GDI+ drawing
    Gdiplus::Bitmap bmp(width, height, stride, PixelFormat32bppPARGB,
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
        renderer_.RenderKeyAction(g, width, height, eventLabel_, anim, config_.keyLabelLayoutMode);
    } else {
        renderer_.RenderPointerAction(g, config_.sizePx, eventKind_, anim, eventLabel_);
    }

    // Commit to layered window
    SIZE sz{ width, height };
    POINT src{ 0, 0 };
    RECT rc{};
    GetWindowRect(targetHwnd, &rc);
    POINT dst{ rc.left, rc.top };
    BLENDFUNCTION bf{};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;
    UpdateLayeredWindow(targetHwnd, ctx.screenDc, &dst, &sz,
                        ctx.memDc, &src, 0, &bf, ULW_ALPHA);
}

// ============================================================================
// Multi-monitor clone management
// ============================================================================

HWND Win32InputIndicatorOverlay::CreateCloneWindow() {
    if (!windowClassRegistered_) {
        // Window class should already be registered by EnsureWindow,
        // but just in case we create clones before the main window.
        if (!EnsureWindow()) return nullptr;
    }

    HWND clone = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kWindowClassName,
        L"",
        WS_POPUP,
        0, 0, renderWidthPx_, renderHeightPx_,
        nullptr, nullptr, GetModuleHandleW(nullptr), this);

    return clone;
}

bool Win32InputIndicatorOverlay::IsCustomMode(bool /*isKeyboard*/) const {
    return config_.targetMonitor == "custom";
}

void Win32InputIndicatorOverlay::TriggerOnEnabledMonitors(
    IndicatorEventKind kind, POINT anchorPt, std::wstring label, bool isKeyboard) {

    // Sync clone windows with enabled monitors
    SyncCloneWindows(isKeyboard);

    // Ensure at least the main window exists for the timer
    if (!EnsureWindow()) return;

    // Show on main window? Only if we don't use clones exclusively.
    // In custom mode we hide the main window and only use clones.
    ShowWindow(hwnd_, SW_HIDE);

    // Set timer on main window to drive animation
    SetTimer(hwnd_, kIndicatorTimerId, 16, nullptr);

    // Show and position each clone
    for (auto& [monId, clone] : cloneWindows_) {
        if (!clone || !IsWindow(clone)) continue;
        UpdateClonePlacement(clone, monId, isKeyboard);
        ShowWindow(clone, SW_SHOWNOACTIVATE);
        RenderToWindow(clone);
    }
}

void Win32InputIndicatorOverlay::UpdateClonePlacement(HWND targetHwnd, const std::string& monitorId, bool /*isKeyboard*/) {
    if (!targetHwnd) return;

    // Find the monitor entry
    auto monitors = mousefx::EnumMonitors();
    RECT monRect{};
    bool found = false;
    for (const auto& m : monitors) {
        if (m.id == monitorId) {
            monRect = m.bounds;
            found = true;
            break;
        }
    }
    if (!found) return;

    // Get position from override or use defaults
    int posX = config_.absoluteX;
    int posY = config_.absoluteY;
    auto it = config_.perMonitorOverrides.find(monitorId);
    if (it != config_.perMonitorOverrides.end()) {
        posX = it->second.absoluteX;
        posY = it->second.absoluteY;
    }

    POINT target{};
    target.x = monRect.left + posX;
    target.y = monRect.top + posY;

    // Clamp to monitor bounds?
    // User requested "allow negative/absolute" support.
    // Removed strict clamping here as well to allow off-screen positioning if desired.
    // The +/- 20000 limit in UpdateConfig prevents extreme values.
    
    SetWindowPos(targetHwnd, HWND_TOPMOST, target.x, target.y,
                 renderWidthPx_, renderHeightPx_, SWP_NOACTIVATE);
}

void Win32InputIndicatorOverlay::SyncCloneWindows(bool /*isKeyboard*/) {
    // Collect enabled monitor IDs
    std::map<std::string, bool> enabledMonitors;
    for (const auto& [id, ov] : config_.perMonitorOverrides) {
        if (ov.enabled) enabledMonitors[id] = true;
    }

    // Remove clones for monitors that are no longer enabled
    for (auto it = cloneWindows_.begin(); it != cloneWindows_.end(); ) {
        if (enabledMonitors.find(it->first) == enabledMonitors.end()) {
            if (it->second && IsWindow(it->second)) {
                DestroyWindow(it->second);
            }
            it = cloneWindows_.erase(it);
        } else {
            ++it;
        }
    }

    // Create clones for newly enabled monitors
    for (const auto& [id, _] : enabledMonitors) {
        if (cloneWindows_.find(id) == cloneWindows_.end()) {
            HWND clone = CreateCloneWindow();
            if (clone) cloneWindows_[id] = clone;
        }
    }
}

void Win32InputIndicatorOverlay::DestroyClones() {
    for (auto& [id, clone] : cloneWindows_) {
        if (clone && IsWindow(clone)) {
            DestroyWindow(clone);
        }
    }
    cloneWindows_.clear();
    customModeActive_ = false;
}

void Win32InputIndicatorOverlay::UpdatePlacement(POINT anchorPt, bool isKeyboard) {
    if (!hwnd_) return;

    // Decide which position parameters to use.
    const std::string& posMode = config_.positionMode;
    const int offX   = config_.offsetX;
    const int offY   = config_.offsetY;
    const int absX   = config_.absoluteX;
    const int absY   = config_.absoluteY;
    const std::string& monId = config_.targetMonitor;

    POINT target{};
    if (IsRelativeMode(posMode)) {
        target.x = anchorPt.x + offX;
        target.y = anchorPt.y + offY;
    } else {
        // Absolute mode: resolve target monitor and place relative to its origin.
        const auto [resolvedMonId, monRect] = ResolveTargetMonitor(monId, anchorPt);
        
        int finalAbsX = absX;
        int finalAbsY = absY;

        // Check for per-monitor override
        if (!resolvedMonId.empty()) {
            auto it = config_.perMonitorOverrides.find(resolvedMonId);
            if (it != config_.perMonitorOverrides.end() && it->second.enabled) {
                finalAbsX = it->second.absoluteX;
                finalAbsY = it->second.absoluteY;
            }
        }

        target.x = monRect.left + finalAbsX;
        target.y = monRect.top  + finalAbsY;
    }

    // Clamp to target monitor bounds? 
    // User requested "allow negative/absolute" support. 
    // We only prevent it from effectively disappearing if the coordinates are wild,
    // but we honor the user's manual offset even if it pushes it off that specific monitor.
    // The UpdateConfig limits (-20000 to 20000) prevent extreme overflow.
    // So we basically trust target.x/y, but maybe ensure it's not effectively invisible?
    // Actually, giving full control is best for "absolute" mode. 
    // We just ensure it respects the 'don't exceed too much' by relying on the +/- 20000 config limit.
    
    // However, to keep it "compatible" and not strictly lost, let's just ensure
    // we don't clamp strictly TO the monitor, but we don't apply extra clamping 
    // beyond the config limits here.
    
    SetWindowPos(hwnd_, HWND_TOPMOST, target.x, target.y, renderWidthPx_, renderHeightPx_, SWP_NOACTIVATE);
}

// ============================================================================
// Utilities
// ============================================================================

int Win32InputIndicatorOverlay::ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

bool Win32InputIndicatorOverlay::IsRelativeMode(const std::string& mode) {
    return mode == "relative";
}

} // namespace mousefx

