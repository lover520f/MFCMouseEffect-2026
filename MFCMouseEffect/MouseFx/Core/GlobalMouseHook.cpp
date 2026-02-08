// GlobalMouseHook.cpp

#include "pch.h"

#include "GlobalMouseHook.h"
#include "MouseFxMessages.h"

#include <new>

namespace mousefx {

GlobalMouseHook* GlobalMouseHook::instance_ = nullptr;

static POINT NormalizeScreenPoint(const POINT& hookPt) {
    // Some virtual display/secondary-screen setups can produce coordinates in a different
    // coordinate space than what our layered windows expect. As a best-effort fallback,
    // prefer GetCursorPos() if the values diverge significantly.
    POINT cursor{};
    if (!GetCursorPos(&cursor)) return hookPt;

    const int dx = cursor.x - hookPt.x;
    const int dy = cursor.y - hookPt.y;
    const int kMismatchPx = 64;
    if ((dx > kMismatchPx) || (dx < -kMismatchPx) || (dy > kMismatchPx) || (dy < -kMismatchPx)) {
        return cursor;
    }
    return hookPt;
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
    return true;
}

void GlobalMouseHook::Stop() {
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
                const POINT pt = NormalizeScreenPoint(s->pt);
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
                // HIWORD of mouseData contains wheel delta
                short delta = static_cast<short>(HIWORD(s->mouseData));
                const POINT pt = NormalizeScreenPoint(s->pt);
                PostMessageW(instance_->dispatchHwnd_, WM_MFX_SCROLL,
                    (WPARAM)delta, MAKELPARAM(pt.x, pt.y));
            }
            break;
        default:
            break;
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
                ev->pt = NormalizeScreenPoint(s->pt);
                ev->button = button;
                PostMessageW(instance_->dispatchHwnd_, WM_MFX_CLICK, 0, reinterpret_cast<LPARAM>(ev));
            }
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace mousefx
