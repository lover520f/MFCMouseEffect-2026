// DispatchRouter.Pointer.cpp -- pointer/button/timer routing paths.

#include "pch.h"

#include "DispatchRouter.h"

#include "DispatchRouter.Internal.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

namespace mousefx {

intptr_t DispatchRouter::OnMove(const DispatchMessage& message) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        return 0;
    }

    ScreenPoint pt{};
    if (!ctrl_->ConsumeLatestMove(&pt)) {
        pt = dispatch_router_detail::MessagePoint(message);
    }
#if MFX_PLATFORM_MACOS
    if (pt.x == 0 && pt.y == 0) {
        ScreenPoint cursor{};
        if (ctrl_->QueryCursorScreenPoint(&cursor)) {
            pt = cursor;
        }
    }
#endif

    automationFeature_.OnMouseMove(*ctrl_, pt);

    IMouseEffect* trailEffect = ctrl_->GetEffect(EffectCategory::Trail);
    const bool forceBuiltinLineTrail = (trailEffect != nullptr) &&
        (NormalizeTrailEffectType(trailEffect->TypeName()) == "line");

    bool moveRenderedByWasm = false;
    bool moveRouteActive = false;
    if (!forceBuiltinLineTrail) {
        moveRouteActive = wasmFeature_.RouteMove(*ctrl_, pt, &moveRenderedByWasm);
    }
    if ((!moveRouteActive || !moveRenderedByWasm) && (trailEffect != nullptr)) {
        if (auto* effect = trailEffect) {
            effect->OnMouseMove(pt);
        }
    }

    wasmFeature_.RouteHoldUpdateIfActive(*ctrl_, pt, static_cast<uint32_t>(ctrl_->CurrentHoldDurationMs()));
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
        effect->OnHoldUpdate(pt, ctrl_->CurrentHoldDurationMs());
    }
    return 0;
}

intptr_t DispatchRouter::OnScroll(const DispatchMessage& message) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        return 0;
    }

    const short delta = static_cast<short>(message.delta);
    ScreenPoint pt{};
    if (!ctrl_->QueryCursorScreenPoint(&pt)) {
        pt = dispatch_router_detail::MessagePoint(message);
    }

    ScrollEvent ev{};
    ev.pt = pt;
    ev.delta = delta;
    ev.horizontal = false;

    automationFeature_.OnScroll(*ctrl_, delta);
    indicatorFeature_.OnScroll(*ctrl_, ev);

    bool scrollRenderedByWasm = false;
    const bool scrollRouteActive = wasmFeature_.RouteScroll(*ctrl_, ev, &scrollRenderedByWasm);
    if ((!scrollRouteActive || !scrollRenderedByWasm) && (ctrl_->GetEffect(EffectCategory::Scroll) != nullptr)) {
        if (auto* effect = ctrl_->GetEffect(EffectCategory::Scroll)) {
            effect->OnScroll(ev);
        }
    }
    return 0;
}

intptr_t DispatchRouter::OnButtonDown(const DispatchMessage& message) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        ctrl_->ClearPendingHold();
        return 0;
    }

    const int button = static_cast<int>(message.button);
    ScreenPoint pt{};
    if (!ctrl_->QueryCursorScreenPoint(&pt)) {
        pt = dispatch_router_detail::MessagePoint(message);
    }

    ctrl_->BeginHoldTracking(pt, button);
    automationFeature_.OnButtonDown(*ctrl_, pt, button);
    ctrl_->ArmHoldTimer();

    return 0;
}

intptr_t DispatchRouter::OnButtonUp(const DispatchMessage& message) {
    ctrl_->EndHoldTracking();
    ctrl_->CancelPendingHold();

    if (ctrl_->IsVmEffectsSuppressed()) {
        automationFeature_.OnSuppressed(*ctrl_);
        return 0;
    }

    ScreenPoint pt{};
    if (!ctrl_->QueryCursorScreenPoint(&pt)) {
        pt.x = 0;
        pt.y = 0;
    }
    automationFeature_.OnButtonUp(*ctrl_, pt, static_cast<int>(message.button));
    wasmFeature_.RouteHoldEndIfActive(*ctrl_, pt);

    if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
        effect->OnHoldEnd();
    }
    return 0;
}

intptr_t DispatchRouter::OnTimer(const DispatchMessage& message) {
    const uintptr_t timerId = static_cast<uintptr_t>(message.timerId);
    if (timerId == AppController::HoverTimerId()) {
        if (ctrl_->IsVmEffectsSuppressed()) {
            return 0;
        }
        ScreenPoint pt{};
        if (ctrl_->TryEnterHover(&pt)) {
            bool hoverRenderedByWasm = false;
            const bool hoverRouteActive = wasmFeature_.RouteHoverStart(*ctrl_, pt, &hoverRenderedByWasm);
            if (!hoverRouteActive || !hoverRenderedByWasm) {
                if (auto* effect = ctrl_->GetEffect(EffectCategory::Hover)) {
                    effect->OnHoverStart(pt);
                }
            }
        }
        return 0;
    }

    if (timerId == AppController::HoldTimerId()) {
        ctrl_->DisarmHoldTimer();
        if (ctrl_->IsVmEffectsSuppressed()) {
            ctrl_->ClearPendingHold();
            return 0;
        }
        ScreenPoint pt{};
        int button = 0;
        if (ctrl_->ConsumePendingHold(&pt, &button)) {
            bool holdRenderedByWasm = false;
            const bool holdRouteActive = wasmFeature_.RouteHoldStart(
                *ctrl_,
                pt,
                button,
                static_cast<uint32_t>(ctrl_->CurrentHoldDurationMs()),
                &holdRenderedByWasm);
            if (!holdRouteActive || !holdRenderedByWasm) {
                if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
                    effect->OnHoldStart(pt, button);
                }
            }
            ctrl_->MarkIgnoreNextClick();
        }
        return 0;
    }

#ifdef _DEBUG
    static constexpr uintptr_t kSelfTestTimerId = 0x4D46;
    if (timerId == kSelfTestTimerId) {
        ctrl_->KillDispatchTimer(kSelfTestTimerId);
        ClickEvent ev{};
        ScreenPoint cursor{};
        if (!ctrl_->QueryCursorScreenPoint(&cursor)) {
            cursor.x = 0;
            cursor.y = 0;
        }
        ev.pt = cursor;
        ev.button = MouseButton::Left;
        if (auto* effect = ctrl_->GetEffect(EffectCategory::Click)) {
            effect->OnClick(ev);
        }
        OutputDebugStringW(L"MouseFx: self-test ripple fired.\n");
        return 0;
    }
#endif

    return 0;
}

} // namespace mousefx
