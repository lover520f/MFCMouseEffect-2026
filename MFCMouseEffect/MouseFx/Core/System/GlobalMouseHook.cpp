// GlobalMouseHook.cpp

#include "pch.h"

#include "GlobalMouseHook.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"

#include <new>
#include <string>

namespace mousefx {

GlobalMouseHook* GlobalMouseHook::instance_ = nullptr;

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

bool GlobalMouseHook::Start(HWND dispatchHwnd) {
    if (hook_ != nullptr) return true;
    if (!IsWindow(dispatchHwnd)) return false;
    if (instance_ != nullptr) return false; // keep it simple: single hook per process

    dispatchHwnd_ = dispatchHwnd;
    instance_ = this;
    lastError_ = ERROR_SUCCESS;
    movePending_.store(false, std::memory_order_release);

    hook_ = SetWindowsHookExW(WH_MOUSE_LL, &GlobalMouseHook::HookProc, GetModuleHandleW(nullptr), 0);
    if (!hook_) {
        lastError_ = GetLastError();
        instance_ = nullptr;
        dispatchHwnd_ = nullptr;
        return false;
    }

    keyboardHook_ = SetWindowsHookExW(WH_KEYBOARD_LL, &GlobalMouseHook::KeyboardHookProc, GetModuleHandleW(nullptr), 0);
#ifdef _DEBUG
    if (!keyboardHook_) {
        wchar_t buf[128]{};
        wsprintfW(buf, L"MouseFx: keyboard hook start failed. GetLastError=%lu\n", GetLastError());
        OutputDebugStringW(buf);
    }
#endif
    return true;
}

void GlobalMouseHook::Stop() {
    if (keyboardHook_) {
        UnhookWindowsHookEx(keyboardHook_);
        keyboardHook_ = nullptr;
    }
    if (hook_) {
        UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
    }
    dispatchHwnd_ = nullptr;
    movePending_.store(false, std::memory_order_release);
    if (instance_ == this) {
        instance_ = nullptr;
    }
    lastError_ = ERROR_SUCCESS;
}

bool GlobalMouseHook::ConsumeLatestMove(POINT& outPt) {
    if (!movePending_.exchange(false, std::memory_order_acq_rel)) {
        return false;
    }
    outPt.x = latestMoveX_.load(std::memory_order_acquire);
    outPt.y = latestMoveY_.load(std::memory_order_acquire);
    return true;
}

LRESULT CALLBACK GlobalMouseHook::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance_ && instance_->dispatchHwnd_) {
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
                    if (!PostMessageW(instance_->dispatchHwnd_, WM_MFX_MOVE, 0, 0)) {
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
            PostMessageW(instance_->dispatchHwnd_, WM_MFX_SCROLL,
                static_cast<WPARAM>(scrollDelta), MAKELPARAM(pt.x, pt.y));
        }

        // Button down event (for Hold detection)
        if (fireButtonDown && s) {
            const POINT pt = NormalizeScreenPoint(s->pt);
            PostMessageW(instance_->dispatchHwnd_, WM_MFX_BUTTON_DOWN,
                (WPARAM)button, MAKELPARAM(pt.x, pt.y));
        }

        // Button up event (triggers click + ends hold)
        if (fireClick && s) {
            // Post button up for Hold end
            PostMessageW(instance_->dispatchHwnd_, WM_MFX_BUTTON_UP,
                (WPARAM)button, 0);

            // Create click event
            auto* ev = new (std::nothrow) ClickEvent();
            if (ev) {
                ev->pt = ResolveCursorPreferredPoint(s->pt);
                ev->button = button;
                PostMessageW(instance_->dispatchHwnd_, WM_MFX_CLICK, 0, reinterpret_cast<LPARAM>(ev));
            }
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK GlobalMouseHook::KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance_ && instance_->dispatchHwnd_) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            const auto* kbd = reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);
            if (kbd) {
                auto* ev = new (std::nothrow) KeyEvent();
                if (ev) {
                    POINT cursor{};
                    if (!GetCursorPos(&cursor)) {
                        cursor.x = 0;
                        cursor.y = 0;
                    }
                    ev->pt = NormalizeScreenPoint(cursor);
                    ev->vkCode = kbd->vkCode;
                    ev->systemKey = (wParam == WM_SYSKEYDOWN);
                    ev->ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                    ev->shift = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) != 0;
                    ev->alt   = (GetAsyncKeyState(VK_MENU)    & 0x8000) != 0;
                    ev->win   = (GetAsyncKeyState(VK_LWIN)    & 0x8000) != 0
                             || (GetAsyncKeyState(VK_RWIN)    & 0x8000) != 0;
                    ev->text = GetKeyDisplayText(kbd);
                    PostMessageW(instance_->dispatchHwnd_, WM_MFX_KEY, 0, reinterpret_cast<LPARAM>(ev));
                }
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace mousefx
