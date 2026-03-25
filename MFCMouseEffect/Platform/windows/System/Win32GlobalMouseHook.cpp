#include "pch.h"

#include "Platform/windows/System/Win32GlobalMouseHook.h"
#include "Platform/windows/Protocol/Win32InputTypes.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"

#include <new>
#include <string>

namespace mousefx {

Win32GlobalMouseHook* Win32GlobalMouseHook::instance_ = nullptr;

namespace {

constexpr uint32_t kModCtrl = 1u << 0;
constexpr uint32_t kModShift = 1u << 1;
constexpr uint32_t kModAlt = 1u << 2;
constexpr uint32_t kModWin = 1u << 3;

bool IsKeyDownMessage(WPARAM wParam) {
    return wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
}

bool IsKeyUpMessage(WPARAM wParam) {
    return wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
}

uint32_t ModifierMaskForVk(UINT vkCode) {
    switch (vkCode) {
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
        return kModCtrl;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
        return kModShift;
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
        return kModAlt;
    case VK_LWIN:
    case VK_RWIN:
        return kModWin;
    default:
        return 0;
    }
}

} // namespace

static POINT ResolveCursorPreferredPoint(const POINT& hookPt) {
    POINT cursor{};
    if (GetCursorPos(&cursor)) {
        return cursor;
    }
    return hookPt;
}

static POINT NormalizeScreenPoint(const POINT& hookPt) {
    return ResolveCursorPreferredPoint(hookPt);
}

static std::wstring FallbackVkName(UINT vkCode) {
    switch (vkCode) {
    case VK_LCONTROL:
    case VK_RCONTROL:
    case VK_CONTROL: return L"Ctrl";
    case VK_LSHIFT:
    case VK_RSHIFT:
    case VK_SHIFT: return L"Shift";
    case VK_LMENU:
    case VK_RMENU:
    case VK_MENU: return L"Alt";
    case VK_LWIN:
    case VK_RWIN: return L"Win";
    case VK_RETURN: return L"Enter";
    case VK_ESCAPE: return L"Esc";
    case VK_BACK: return L"Backspace";
    case VK_TAB: return L"Tab";
    case VK_SPACE: return L"Space";
    case VK_DELETE: return L"Delete";
    case VK_UP: return L"Up";
    case VK_DOWN: return L"Down";
    case VK_LEFT: return L"Left";
    case VK_RIGHT: return L"Right";
    default:
        break;
    }

    if (vkCode >= 'A' && vkCode <= 'Z') {
        return std::wstring(1, static_cast<wchar_t>(vkCode));
    }
    if (vkCode >= '0' && vkCode <= '9') {
        return std::wstring(1, static_cast<wchar_t>(vkCode));
    }

    wchar_t buf[16]{};
    wsprintfW(buf, L"VK_%02X", static_cast<unsigned>(vkCode));
    return std::wstring(buf);
}

static std::wstring GetKeyDisplayText(const KBDLLHOOKSTRUCT* kbd) {
    if (!kbd) return {};
    LONG lParam = static_cast<LONG>(kbd->scanCode << 16);
    if ((kbd->flags & LLKHF_EXTENDED) != 0) lParam |= (1 << 24);
    wchar_t keyName[64]{};
    const int len = GetKeyNameTextW(lParam, keyName, static_cast<int>(std::size(keyName)));
    if (len > 0) {
        return std::wstring(keyName, keyName + len);
    }
    return FallbackVkName(kbd->vkCode);
}

bool Win32GlobalMouseHook::Start(IDispatchMessageHost* dispatchHost) {
    if (hook_ != nullptr) return true;
    if (!dispatchHost) return false;
    if (instance_ != nullptr) return false;

    dispatchHost_ = dispatchHost;
    instance_ = this;
    lastError_ = ERROR_SUCCESS;
    movePending_.store(false, std::memory_order_release);
    keyboardCaptureExclusive_.store(false, std::memory_order_release);
    keyboardModifierMask_.store(0, std::memory_order_release);

    hook_ = SetWindowsHookExW(WH_MOUSE_LL, &Win32GlobalMouseHook::HookProc, GetModuleHandleW(nullptr), 0);
    if (!hook_) {
        lastError_ = GetLastError();
        instance_ = nullptr;
        dispatchHost_ = nullptr;
        return false;
    }

    keyboardHook_ = SetWindowsHookExW(WH_KEYBOARD_LL, &Win32GlobalMouseHook::KeyboardHookProc, GetModuleHandleW(nullptr), 0);
#ifdef _DEBUG
    if (!keyboardHook_) {
        wchar_t buf[128]{};
        wsprintfW(buf, L"MouseFx: keyboard hook start failed. GetLastError=%lu\n", GetLastError());
        OutputDebugStringW(buf);
    }
#endif
    return true;
}

void Win32GlobalMouseHook::Stop() {
    if (keyboardHook_) {
        UnhookWindowsHookEx(keyboardHook_);
        keyboardHook_ = nullptr;
    }
    if (hook_) {
        UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
    }
    dispatchHost_ = nullptr;
    movePending_.store(false, std::memory_order_release);
    keyboardCaptureExclusive_.store(false, std::memory_order_release);
    keyboardModifierMask_.store(0, std::memory_order_release);
    if (instance_ == this) {
        instance_ = nullptr;
    }
    lastError_ = ERROR_SUCCESS;
}

bool Win32GlobalMouseHook::ConsumeLatestMove(ScreenPoint& outPt) {
    if (!movePending_.exchange(false, std::memory_order_acq_rel)) {
        return false;
    }
    outPt.x = latestMoveX_.load(std::memory_order_acquire);
    outPt.y = latestMoveY_.load(std::memory_order_acquire);
    return true;
}

void Win32GlobalMouseHook::SetKeyboardCaptureExclusive(bool enabled) {
    keyboardCaptureExclusive_.store(enabled, std::memory_order_release);
}

LRESULT CALLBACK Win32GlobalMouseHook::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance_ && instance_->dispatchHost_) {
        const auto* s = reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);

        MouseButton button{};
        bool fireClick = false;
        bool fireButtonDown = false;
        bool fireScroll = false;
        short scrollDelta = 0;

        switch (wParam) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            button = MouseButton::Left;
            fireButtonDown = true;
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            button = MouseButton::Right;
            fireButtonDown = true;
            break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
            button = MouseButton::Middle;
            fireButtonDown = true;
            break;
        case WM_LBUTTONUP:
            button = MouseButton::Left;
            fireClick = true;
            break;
        case WM_RBUTTONUP:
            button = MouseButton::Right;
            fireClick = true;
            break;
        case WM_MBUTTONUP:
            button = MouseButton::Middle;
            fireClick = true;
            break;
        case WM_MOUSEMOVE:
            if (s) {
                const POINT pt = ResolveCursorPreferredPoint(s->pt);
                instance_->latestMoveX_.store(pt.x, std::memory_order_release);
                instance_->latestMoveY_.store(pt.y, std::memory_order_release);
                if (!instance_->movePending_.exchange(true, std::memory_order_acq_rel)) {
                    if (!instance_->dispatchHost_->PostAsync(
                            WM_MFX_MOVE,
                            static_cast<uintptr_t>(pt.x),
                            static_cast<intptr_t>(pt.y))) {
                        instance_->movePending_.store(false, std::memory_order_release);
                    }
                }
            }
            break;
        case WM_MOUSEWHEEL:
            if (s) {
                scrollDelta = static_cast<short>(HIWORD(s->mouseData));
                fireScroll = (scrollDelta != 0);
            }
            break;
        case WM_MOUSEHWHEEL:
            if (s) {
                scrollDelta = static_cast<short>(HIWORD(s->mouseData));
                fireScroll = (scrollDelta != 0);
            }
            break;
        default:
            break;
        }

        if (fireScroll && s) {
            const POINT pt = NormalizeScreenPoint(s->pt);
            uint32_t scrollFlags = 0;
            if (wParam == WM_MOUSEHWHEEL) {
                scrollFlags |= kScrollDispatchFlagHorizontal;
            }
            instance_->dispatchHost_->PostAsync(
                WM_MFX_SCROLL,
                static_cast<WPARAM>(PackScrollDispatchPayload(static_cast<int32_t>(scrollDelta), scrollFlags)),
                MAKELPARAM(pt.x, pt.y));
        }

        if (fireButtonDown && s) {
            const POINT pt = NormalizeScreenPoint(s->pt);
            instance_->dispatchHost_->PostAsync(
                WM_MFX_BUTTON_DOWN,
                static_cast<WPARAM>(button),
                MAKELPARAM(pt.x, pt.y));
        }

        if (fireClick && s) {
            instance_->dispatchHost_->PostAsync(
                WM_MFX_BUTTON_UP,
                static_cast<WPARAM>(button),
                0);

            auto* ev = new (std::nothrow) ClickEvent();
            if (ev) {
                ev->pt = ToScreenPoint(ResolveCursorPreferredPoint(s->pt));
                ev->button = button;
                if (!instance_->dispatchHost_->PostAsync(
                        WM_MFX_CLICK,
                        0,
                        reinterpret_cast<LPARAM>(ev))) {
                    delete ev;
                }
            }
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK Win32GlobalMouseHook::KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance_ && instance_->dispatchHost_) {
        const bool keyDown = IsKeyDownMessage(wParam);
        const bool keyUp = IsKeyUpMessage(wParam);
        const bool captureExclusive = instance_->keyboardCaptureExclusive_.load(std::memory_order_acquire);

        const auto* kbd = reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
        if (kbd) {
            const uint32_t modifierBit = ModifierMaskForVk(kbd->vkCode);
            if (modifierBit != 0 && (keyDown || keyUp)) {
                uint32_t mask = instance_->keyboardModifierMask_.load(std::memory_order_acquire);
                if (keyDown) {
                    mask |= modifierBit;
                } else {
                    mask &= ~modifierBit;
                }
                instance_->keyboardModifierMask_.store(mask, std::memory_order_release);
            }

            if (keyDown || keyUp) {
                const uint32_t mask = instance_->keyboardModifierMask_.load(std::memory_order_acquire);
                auto* ev = new (std::nothrow) KeyEvent();
                if (ev) {
                    POINT cursor{};
                    if (!GetCursorPos(&cursor)) {
                        cursor.x = 0;
                        cursor.y = 0;
                    }
                    ev->pt = ToScreenPoint(NormalizeScreenPoint(cursor));
                    ev->vkCode = static_cast<uint32_t>(kbd->vkCode);
                    ev->keyDown = keyDown;
                    ev->systemKey = (wParam == WM_SYSKEYDOWN || wParam == WM_SYSKEYUP);
                    ev->ctrl = (mask & kModCtrl) != 0;
                    ev->shift = (mask & kModShift) != 0;
                    ev->alt = (mask & kModAlt) != 0 || (kbd->flags & LLKHF_ALTDOWN) != 0;
                    ev->win = (mask & kModWin) != 0;
                    ev->meta = ev->win;
                    ev->text = GetKeyDisplayText(kbd);
                    if (!instance_->dispatchHost_->PostAsync(
                            WM_MFX_KEY,
                            0,
                            reinterpret_cast<LPARAM>(ev))) {
                        delete ev;
                    }
                }
            }
        }
        if (captureExclusive && (keyDown || keyUp)) {
            return 1;
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace mousefx
