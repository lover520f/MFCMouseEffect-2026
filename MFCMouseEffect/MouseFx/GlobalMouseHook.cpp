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
        bool fire = false;
        switch (wParam) {
        case WM_LBUTTONUP:
            button = MouseButton::Left;
            fire = true;
            break;
        case WM_RBUTTONUP:
            button = MouseButton::Right;
            fire = true;
            break;
        case WM_MBUTTONUP:
            button = MouseButton::Middle;
            fire = true;
            break;
        case WM_MOUSEMOVE:
        {
            // Forward mouse move.
            // Note: WH_MOUSE_LL callback is called in context of installing thread.
            // PostMessage is safe.
            // Pack coordinates into wParam (x) and lParam (y) since they fit in 64-bit pointers?
            // Actually on x64, WPARAM and LPARAM are 64-bit.
            // POINT.x is LONG (32-bit signed). 
            if (s) {
                 PostMessageW(instance_->dispatchHwnd_, WM_MFX_MOVE, (WPARAM)s->pt.x, (LPARAM)s->pt.y);
            }
            break;
        }
        default:
            break;
        }

        if (fire && s) {
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
