// DispatchRouter.cpp -- Win32 message routing extracted from AppController

#include "pch.h"
#include "DispatchRouter.h"
#include "AppController.h"
#include "MouseFxMessages.h"

#include <windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM
#include <algorithm>

namespace mousefx {

DispatchRouter::DispatchRouter(AppController* controller)
    : ctrl_(controller) {}

LRESULT DispatchRouter::Route(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    const bool isMouseInputMsg =
        (msg == WM_MFX_CLICK || msg == WM_MFX_MOVE || msg == WM_MFX_SCROLL ||
         msg == WM_MFX_BUTTON_DOWN || msg == WM_MFX_BUTTON_UP || msg == WM_MFX_KEY);
    const bool isStateTimerMsg =
        (msg == WM_TIMER && (wParam == ctrl_->kHoverTimerId || wParam == ctrl_->kHoldTimerId));
    if (isMouseInputMsg || isStateTimerMsg) {
        ctrl_->UpdateVmSuppressionState();
    }

    // Reset idle timer on any mouse input
    if (isMouseInputMsg) {
        ctrl_->lastInputTime_ = GetTickCount64();
        if (ctrl_->hovering_) {
            ctrl_->hovering_ = false;
            if (auto* effect = ctrl_->GetEffect(EffectCategory::Hover)) {
                effect->OnHoverEnd();
            }
        }
    }

    switch (msg) {
        case WM_MFX_CLICK:      return OnClick(hwnd, lParam);
        case WM_MFX_MOVE:       return OnMove(hwnd, wParam, lParam);
        case WM_MFX_SCROLL:     return OnScroll(hwnd, wParam, lParam);
        case WM_MFX_KEY:        return OnKey(hwnd, lParam);
        case WM_MFX_BUTTON_DOWN:return OnButtonDown(hwnd, wParam, lParam);
        case WM_MFX_BUTTON_UP:  return OnButtonUp(hwnd, wParam, lParam);
        case WM_TIMER:          return OnTimer(hwnd, wParam);

        case WM_MFX_EXEC_CMD: {
            auto* cmdStr = reinterpret_cast<const std::string*>(lParam);
            if (cmdStr) {
                ctrl_->HandleCommand(*cmdStr);
            }
            return 0;
        }
        case WM_MFX_GET_CONFIG: {
            auto* out = reinterpret_cast<EffectConfig*>(lParam);
            if (out) {
                *out = ctrl_->config_;
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

LRESULT DispatchRouter::OnClick(HWND /*hwnd*/, LPARAM lParam) {
    auto* ev = reinterpret_cast<ClickEvent*>(lParam);
    if (ctrl_->vmEffectsSuppressed_) {
        if (ev) delete ev;
        return 0;
    }
    if (ctrl_->ignoreNextClick_) {
        ctrl_->ignoreNextClick_ = false;
        if (ev) delete ev;
        return 0;  // Suppress click after a long hold
    }

    if (ev) {
        ctrl_->inputIndicatorOverlay_.OnClick(*ev);
#ifdef _DEBUG
        if (ctrl_->debugClickCount_ < 5) {
            ctrl_->debugClickCount_++;
            wchar_t buf[256]{};
            wsprintfW(buf, L"MouseFx: click received (%u) pt=(%ld,%ld) button=%u\n",
                ctrl_->debugClickCount_, ev->pt.x, ev->pt.y, (unsigned)ev->button);
            OutputDebugStringW(buf);
        }
#endif
        if (auto* effect = ctrl_->GetEffect(EffectCategory::Click)) {
            effect->OnClick(*ev);
        }
        delete ev;
    }
    return 0;
}

LRESULT DispatchRouter::OnMove(HWND /*hwnd*/, WPARAM wParam, LPARAM lParam) {
    if (ctrl_->vmEffectsSuppressed_) {
        return 0;
    }
    POINT pt{};
    if (!ctrl_->hook_.ConsumeLatestMove(pt)) {
        pt.x = static_cast<LONG>(wParam);
        pt.y = static_cast<LONG>(lParam);
    }
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Trail)) {
        effect->OnMouseMove(pt);
    }
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
        DWORD holdMs = 0;
        if (ctrl_->holdButtonDown_ && ctrl_->holdDownTick_ != 0) {
            const uint64_t now = GetTickCount64();
            const uint64_t delta = (now >= ctrl_->holdDownTick_) ? (now - ctrl_->holdDownTick_) : 0;
            holdMs = (DWORD)std::min<uint64_t>(delta, 0xFFFFFFFFu);
        }
        effect->OnHoldUpdate(pt, holdMs);
    }
    return 0;
}

LRESULT DispatchRouter::OnScroll(HWND /*hwnd*/, WPARAM wParam, LPARAM lParam) {
    if (ctrl_->vmEffectsSuppressed_) {
        return 0;
    }
    short delta = static_cast<short>(wParam);
    POINT pt{};
    if (!GetCursorPos(&pt)) {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    }
    ScrollEvent ev{};
    ev.pt = pt;
    ev.delta = delta;
    ev.horizontal = false;
    ctrl_->inputIndicatorOverlay_.OnScroll(ev);
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Scroll)) {
        effect->OnScroll(ev);
    }
    return 0;
}

LRESULT DispatchRouter::OnKey(HWND /*hwnd*/, LPARAM lParam) {
    auto* ev = reinterpret_cast<KeyEvent*>(lParam);
    if (ctrl_->vmEffectsSuppressed_) {
        if (ev) delete ev;
        return 0;
    }
    if (ev) {
        ctrl_->inputIndicatorOverlay_.OnKey(*ev);
        delete ev;
    }
    return 0;
}

LRESULT DispatchRouter::OnButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    if (ctrl_->vmEffectsSuppressed_) {
        ctrl_->pendingHold_.active = false;
        return 0;
    }
    int button = static_cast<int>(wParam);
    POINT pt{};
    if (!GetCursorPos(&pt)) {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    }

    ctrl_->holdButtonDown_ = true;
    ctrl_->holdDownTick_ = GetTickCount64();

    // Start delayed hold
    ctrl_->pendingHold_.pt = pt;
    ctrl_->pendingHold_.button = button;
    ctrl_->pendingHold_.active = true;
    ctrl_->ignoreNextClick_ = false;  // Reset for new interaction
    SetTimer(hwnd, ctrl_->kHoldTimerId, ctrl_->kHoldDelayMs, nullptr);

    return 0;
}

LRESULT DispatchRouter::OnButtonUp(HWND hwnd, WPARAM /*wParam*/, LPARAM /*lParam*/) {
    ctrl_->holdButtonDown_ = false;
    ctrl_->holdDownTick_ = 0;

    // Cancel pending hold if quick click
    if (ctrl_->pendingHold_.active) {
        KillTimer(hwnd, ctrl_->kHoldTimerId);
        ctrl_->pendingHold_.active = false;
    }

    if (ctrl_->vmEffectsSuppressed_) {
        return 0;
    }

    if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
        effect->OnHoldEnd();
    }
    return 0;
}

LRESULT DispatchRouter::OnTimer(HWND hwnd, WPARAM wParam) {
    if (wParam == ctrl_->kHoverTimerId) {
        if (ctrl_->vmEffectsSuppressed_) {
            return 0;
        }
        if (!ctrl_->hovering_) {
            uint64_t elapsed = GetTickCount64() - ctrl_->lastInputTime_;
            if (elapsed >= ctrl_->kHoverThresholdMs) {
                ctrl_->hovering_ = true;
                if (auto* effect = ctrl_->GetEffect(EffectCategory::Hover)) {
                    POINT pt;
                    GetCursorPos(&pt);
                    effect->OnHoverStart(pt);
                }
            }
        }
        return 0;
    }

    if (wParam == ctrl_->kHoldTimerId) {
        KillTimer(hwnd, ctrl_->kHoldTimerId);
        if (ctrl_->vmEffectsSuppressed_) {
            ctrl_->pendingHold_.active = false;
            return 0;
        }
        if (ctrl_->pendingHold_.active) {
            if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
                effect->OnHoldStart(ctrl_->pendingHold_.pt, ctrl_->pendingHold_.button);
            }
            ctrl_->pendingHold_.active = false;
            ctrl_->ignoreNextClick_ = true;  // Timer fired = Hold triggered = Ignore next click
        }
        return 0;
    }

#ifdef _DEBUG
    static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;
    if (wParam == kSelfTestTimerId) {
        KillTimer(ctrl_->dispatchHwnd_, kSelfTestTimerId);
        ClickEvent ev{};
        GetCursorPos(&ev.pt);
        ev.button = MouseButton::Left;
        if (auto* effect = ctrl_->GetEffect(EffectCategory::Click)) {
            effect->OnClick(ev);
        }
        OutputDebugStringW(L"MouseFx: self-test ripple fired.\n");
        return 0;
    }
#endif

    return DefWindowProcW(hwnd, WM_TIMER, wParam, 0);
}

} // namespace mousefx
