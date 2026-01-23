// GlobalMouseHook.cpp

#include "pch.h"

#include "GlobalMouseHook.h"
#include "MouseFxMessages.h"

#include <new>

namespace mousefx {

GlobalMouseHook* GlobalMouseHook::instance_ = nullptr;

bool GlobalMouseHook::Start(HWND dispatchHwnd) {
    if (hook_ != nullptr) return true;
    if (!IsWindow(dispatchHwnd)) return false;
    if (instance_ != nullptr) return false; // keep it simple: single hook per process

    dispatchHwnd_ = dispatchHwnd;
    instance_ = this;
    lastError_ = ERROR_SUCCESS;

    hook_ = SetWindowsHookExW(WH_MOUSE_LL, &GlobalMouseHook::HookProc, GetModuleHandleW(nullptr), 0);
    if (!hook_) {
        lastError_ = GetLastError();
        instance_ = nullptr;
        dispatchHwnd_ = nullptr;
        return false;
    }
    return true;
}

void GlobalMouseHook::Stop() {
    if (hook_) {
        UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
    }
    dispatchHwnd_ = nullptr;
    if (instance_ == this) {
        instance_ = nullptr;
    }
    lastError_ = ERROR_SUCCESS;
}

LRESULT CALLBACK GlobalMouseHook::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance_ && instance_->dispatchHwnd_) {
        const auto* s = reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);

        MouseButton button{};
        bool fireClick = false;
        bool fireButtonDown = false;

        switch (wParam) {
        case WM_LBUTTONDOWN:
            button = MouseButton::Left;
            fireButtonDown = true;
            break;
        case WM_RBUTTONDOWN:
            button = MouseButton::Right;
            fireButtonDown = true;
            break;
        case WM_MBUTTONDOWN:
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
                PostMessageW(instance_->dispatchHwnd_, WM_MFX_MOVE, 
                    (WPARAM)s->pt.x, (LPARAM)s->pt.y);
            }
            break;
        case WM_MOUSEWHEEL:
            if (s) {
                // HIWORD of mouseData contains wheel delta
                short delta = static_cast<short>(HIWORD(s->mouseData));
                PostMessageW(instance_->dispatchHwnd_, WM_MFX_SCROLL,
                    (WPARAM)delta, MAKELPARAM(s->pt.x, s->pt.y));
            }
            break;
        default:
            break;
        }

        // Button down event (for Hold detection)
        if (fireButtonDown && s) {
            PostMessageW(instance_->dispatchHwnd_, WM_MFX_BUTTON_DOWN,
                (WPARAM)button, MAKELPARAM(s->pt.x, s->pt.y));
        }

        // Button up event (triggers click + ends hold)
        if (fireClick && s) {
            // Post button up for Hold end
            PostMessageW(instance_->dispatchHwnd_, WM_MFX_BUTTON_UP,
                (WPARAM)button, 0);

            // Create click event
            auto* ev = new (std::nothrow) ClickEvent();
            if (ev) {
                ev->pt = s->pt;
                ev->button = button;
                PostMessageW(instance_->dispatchHwnd_, WM_MFX_CLICK, 0, reinterpret_cast<LPARAM>(ev));
            }
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace mousefx

