// DispatchRouter.cpp -- platform-neutral dispatch routing.

#include "pch.h"

#include "DispatchRouter.h"

#include "DispatchRouter.Internal.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

namespace mousefx {
namespace {

bool IsHoldInteractionActive(AppController* controller) {
    if (!controller) {
        return false;
    }
    return controller->CurrentHoldDurationMs() >= AppController::HoldDelayMs();
}

bool ShouldSuppressClickEffectForHoldPolicy(AppController* controller) {
    return IsHoldInteractionActive(controller);
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

    ctrl_->OnDispatchActivity(message.kind, message.timerId);

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
        if (!dispatch_router_detail::IsKnownTimerId(timerId)) {
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
        ctrl_->RememberLastPointerPoint(ev->pt);
        const bool suppressClickEffectByPolicy = ShouldSuppressClickEffectForHoldPolicy(ctrl_);
        const IMouseEffect* clickEffect = ctrl_->GetEffect(EffectCategory::Click);
        const bool effectIsText = (clickEffect != nullptr) &&
            (NormalizeClickEffectType(clickEffect->TypeName()) == "text");
        const bool configIsText =
            (NormalizeClickEffectType(ctrl_->Config().active.click) == "text");
        const bool forceBuiltinTextClick = effectIsText || configIsText;
        bool renderedByWasm = false;
        bool wasmRouteActive = false;
        if (!forceBuiltinTextClick && !suppressClickEffectByPolicy) {
            wasmRouteActive = wasmFeature_.RouteClick(*ctrl_, *ev, &renderedByWasm);
        }
        automationFeature_.OnClick(*ctrl_, *ev);
        indicatorFeature_.OnClick(*ctrl_, *ev);
        ctrl_->LogDebugClick(*ev);
        const bool shouldFallbackToBuiltin =
            forceBuiltinTextClick ||
            ((!wasmRouteActive) || ctrl_->ShouldFallbackToBuiltinClickWhenWasmActive());
        if (!suppressClickEffectByPolicy &&
            shouldFallbackToBuiltin &&
            (forceBuiltinTextClick || !renderedByWasm)) {
            if (auto* effect = ctrl_->GetEffect(EffectCategory::Click)) {
                effect->OnClick(*ev);
            }
        }
        delete ev;
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

} // namespace mousefx
