// DispatchRouter.cpp -- platform-neutral dispatch routing.

#include "pch.h"

#include "DispatchRouter.h"

#include "AppController.h"

namespace mousefx {

namespace {

ScreenPoint MessagePoint(const DispatchMessage& message) {
    ScreenPoint pt{};
    pt.x = message.x;
    pt.y = message.y;
    return pt;
}

bool IsKnownTimerId(uintptr_t timerId) {
    if (timerId == AppController::HoverTimerId()) {
        return true;
    }
    if (timerId == AppController::HoldTimerId()) {
        return true;
    }
#ifdef _DEBUG
    static constexpr uintptr_t kSelfTestTimerId = 0x4D46;
    if (timerId == kSelfTestTimerId) {
        return true;
    }
#endif
    return false;
}

} // namespace

DispatchRouter::DispatchRouter(AppController* controller)
    : ctrl_(controller) {}

intptr_t DispatchRouter::Route(const DispatchMessage& message, bool* outHandled) {
    if (!ctrl_) {
        if (outHandled) {
            *outHandled = false;
        }
        return 0;
    }

    ctrl_->OnDispatchActivity(
        static_cast<uint32_t>(message.nativeMsg),
        static_cast<uintptr_t>(message.nativeWParam));

    if (outHandled) {
        *outHandled = true;
    }

    switch (message.kind) {
    case DispatchMessageKind::Click:
        return OnClick(message);
    case DispatchMessageKind::Move:
        return OnMove(message);
    case DispatchMessageKind::Scroll:
        return OnScroll(message);
    case DispatchMessageKind::Key:
        return OnKey(message);
    case DispatchMessageKind::ButtonDown:
        return OnButtonDown(message);
    case DispatchMessageKind::ButtonUp:
        return OnButtonUp(message);
    case DispatchMessageKind::Timer: {
        const uintptr_t timerId = static_cast<uintptr_t>(message.timerId);
        if (!IsKnownTimerId(timerId)) {
            if (outHandled) {
                *outHandled = false;
            }
            return 0;
        }
        return OnTimer(message);
    }
    case DispatchMessageKind::ExecCmd:
        if (message.commandJson) {
            ctrl_->HandleCommand(*message.commandJson);
        }
        return 0;
    case DispatchMessageKind::GetConfig:
        if (message.configOut) {
            *message.configOut = ctrl_->Config();
        }
        return 0;
    case DispatchMessageKind::Unknown:
    default:
        if (outHandled) {
            *outHandled = false;
        }
        return 0;
    }
}

intptr_t DispatchRouter::OnClick(const DispatchMessage& message) {
    ClickEvent* ev = message.clickEvent;
    if (ctrl_->IsVmEffectsSuppressed()) {
        if (ev) delete ev;
        return 0;
    }
    if (ctrl_->ConsumeIgnoreNextClick()) {
        if (ev) delete ev;
        return 0;
    }

    if (ev) {
        bool renderedByWasm = false;
        const bool wasmRouteActive = wasmFeature_.RouteClick(*ctrl_, *ev, &renderedByWasm);
        automationFeature_.OnClick(*ctrl_, *ev);
        indicatorFeature_.OnClick(*ctrl_, *ev);
        ctrl_->LogDebugClick(*ev);
        const bool shouldFallbackToBuiltin =
            (!wasmRouteActive) || ctrl_->ShouldFallbackToBuiltinClickWhenWasmActive();
        if (!renderedByWasm && shouldFallbackToBuiltin) {
            if (auto* effect = ctrl_->GetEffect(EffectCategory::Click)) {
                effect->OnClick(*ev);
            }
        }
        delete ev;
    }
    return 0;
}

intptr_t DispatchRouter::OnMove(const DispatchMessage& message) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        return 0;
    }

    ScreenPoint pt{};
    if (!ctrl_->ConsumeLatestMove(&pt)) {
        pt = MessagePoint(message);
    }

    automationFeature_.OnMouseMove(*ctrl_, pt);

    bool moveRenderedByWasm = false;
    const bool moveRouteActive = wasmFeature_.RouteMove(*ctrl_, pt, &moveRenderedByWasm);
    if ((!moveRouteActive || !moveRenderedByWasm) && (ctrl_->GetEffect(EffectCategory::Trail) != nullptr)) {
        if (auto* effect = ctrl_->GetEffect(EffectCategory::Trail)) {
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
        pt = MessagePoint(message);
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

intptr_t DispatchRouter::OnKey(const DispatchMessage& message) {
    KeyEvent* ev = message.keyEvent;
    if (ctrl_->IsVmEffectsSuppressed()) {
        if (ev) delete ev;
        return 0;
    }
    if (ev) {
        ctrl_->OnGlobalKey(*ev);
        delete ev;
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
        pt = MessagePoint(message);
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

