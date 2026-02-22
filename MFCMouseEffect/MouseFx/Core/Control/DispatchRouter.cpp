// DispatchRouter.cpp -- platform-neutral dispatch routing.

#include "pch.h"

#include "DispatchRouter.h"

#include "AppController.h"
#include "MouseFx/Core/Wasm/WasmClickCommandExecutor.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"

#include <vector>

namespace mousefx {

namespace {

uint8_t ToWasmButton(MouseButton button) {
    switch (button) {
    case MouseButton::Left:
        return 1;
    case MouseButton::Right:
        return 2;
    case MouseButton::Middle:
        return 3;
    default:
        return 0;
    }
}

uint8_t ToWasmButtonFromCode(int button) {
    switch (button) {
    case 1:
        return 1;
    case 2:
        return 2;
    case 3:
        return 3;
    default:
        return 0;
    }
}

ScreenPoint MessagePoint(const DispatchMessage& message) {
    ScreenPoint pt{};
    pt.x = message.x;
    pt.y = message.y;
    return pt;
}

bool TryReadCursorScreenPoint(ScreenPoint* outPt) {
    if (!outPt) {
        return false;
    }
    POINT nativePt{};
    if (!GetCursorPos(&nativePt)) {
        return false;
    }
    outPt->x = nativePt.x;
    outPt->y = nativePt.y;
    return true;
}

bool IsKnownTimerId(UINT_PTR timerId) {
    if (timerId == AppController::HoverTimerId()) {
        return true;
    }
    if (timerId == AppController::HoldTimerId()) {
        return true;
    }
#ifdef _DEBUG
    static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;
    if (timerId == kSelfTestTimerId) {
        return true;
    }
#endif
    return false;
}

} // namespace

DispatchRouter::DispatchRouter(AppController* controller)
    : ctrl_(controller) {}

bool DispatchRouter::TryInvokeAndRenderWasmEvent(
    const wasm::EventInvokeInput& input,
    bool* outRenderedByWasm,
    bool* outInvokeOk) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (outInvokeOk) {
        *outInvokeOk = false;
    }
    if (!ctrl_) {
        return false;
    }
    auto* wasmHost = ctrl_->WasmHost();
    if (!wasmHost || !wasmHost->Enabled() || !wasmHost->IsPluginLoaded()) {
        return false;
    }

    std::vector<uint8_t> commandBuffer;
    const bool wasmOk = wasmHost->InvokeEvent(input, &commandBuffer);
    if (outInvokeOk) {
        *outInvokeOk = wasmOk;
    }

    wasm::CommandExecutionResult execResult{};
    bool renderedByWasm = false;
    if (wasmOk && !commandBuffer.empty()) {
        const std::wstring manifestPath = wasmHost->Diagnostics().activeManifestPath;
        execResult = wasm::WasmClickCommandExecutor::Execute(
            commandBuffer.data(),
            commandBuffer.size(),
            ctrl_->Config(),
            manifestPath);
        renderedByWasm = execResult.renderedAny;
    }
    wasmHost->RecordRenderExecution(
        renderedByWasm,
        execResult.executedTextCommands,
        execResult.executedImageCommands,
        execResult.droppedCommands,
        execResult.lastError);

    if (outRenderedByWasm) {
        *outRenderedByWasm = renderedByWasm;
    }

#ifdef _DEBUG
    const wasm::HostDiagnostics& diag = wasmHost->Diagnostics();
    wchar_t buffer[288]{};
    wsprintfW(
        buffer,
        L"MouseFx: wasm_event kind=%u ok=%d bytes=%lu commands=%lu parse=%hs err=%hs rendered=%d text=%lu image=%lu drop=%lu\n",
        static_cast<unsigned>(input.kind),
        wasmOk ? 1 : 0,
        static_cast<unsigned long>(diag.lastOutputBytes),
        static_cast<unsigned long>(diag.lastCommandCount),
        wasm::CommandParseErrorToString(diag.lastParseError),
        diag.lastError.c_str(),
        renderedByWasm ? 1 : 0,
        static_cast<unsigned long>(execResult.executedTextCommands),
        static_cast<unsigned long>(execResult.executedImageCommands),
        static_cast<unsigned long>(execResult.droppedCommands));
    OutputDebugStringW(buffer);
#endif

    return true;
}

intptr_t DispatchRouter::Route(const DispatchMessage& message, bool* outHandled) {
    if (!ctrl_) {
        if (outHandled) {
            *outHandled = false;
        }
        return 0;
    }

    ctrl_->OnDispatchActivity(
        static_cast<UINT>(message.nativeMsg),
        static_cast<WPARAM>(message.nativeWParam));

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
        const UINT_PTR timerId = static_cast<UINT_PTR>(message.timerId);
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
        bool wasmRouteActive = false;
        if (auto* wasmHost = ctrl_->WasmHost()) {
            wasmRouteActive = wasmHost->Enabled() && wasmHost->IsPluginLoaded();
        }
        if (wasmRouteActive) {
            wasm::EventInvokeInput invoke{};
            invoke.kind = wasm::EventKind::Click;
            invoke.x = ev->pt.x;
            invoke.y = ev->pt.y;
            invoke.button = ToWasmButton(ev->button);
            invoke.eventTickMs = GetTickCount64();
            bool invokeOk = false;
            TryInvokeAndRenderWasmEvent(invoke, &renderedByWasm, &invokeOk);
        }
        ctrl_->InputAutomation().OnClick(*ev);
        ctrl_->IndicatorOverlay().OnClick(*ev);
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

    ctrl_->InputAutomation().OnMouseMove(pt);

    bool moveRenderedByWasm = false;
    bool moveRouteActive = false;
    if (auto* wasmHost = ctrl_->WasmHost()) {
        moveRouteActive = wasmHost->Enabled() && wasmHost->IsPluginLoaded();
    }
    if (moveRouteActive) {
        wasm::EventInvokeInput moveInvoke{};
        moveInvoke.kind = wasm::EventKind::Move;
        moveInvoke.x = pt.x;
        moveInvoke.y = pt.y;
        moveInvoke.eventTickMs = GetTickCount64();
        bool invokeOk = false;
        TryInvokeAndRenderWasmEvent(moveInvoke, &moveRenderedByWasm, &invokeOk);
    }
    if ((!moveRouteActive || !moveRenderedByWasm) && (ctrl_->GetEffect(EffectCategory::Trail) != nullptr)) {
        if (auto* effect = ctrl_->GetEffect(EffectCategory::Trail)) {
            effect->OnMouseMove(pt);
        }
    }

    if (wasmHoldEventActive_) {
        wasm::EventInvokeInput holdInvoke{};
        holdInvoke.kind = wasm::EventKind::HoldUpdate;
        holdInvoke.x = pt.x;
        holdInvoke.y = pt.y;
        holdInvoke.button = wasmHoldButton_;
        holdInvoke.holdMs = static_cast<uint32_t>(ctrl_->CurrentHoldDurationMs());
        holdInvoke.eventTickMs = GetTickCount64();
        bool renderedByWasm = false;
        bool invokeOk = false;
        TryInvokeAndRenderWasmEvent(holdInvoke, &renderedByWasm, &invokeOk);
        if (!invokeOk) {
            wasmHoldEventActive_ = false;
            wasmHoldButton_ = 0;
        }
    }
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
    if (!TryReadCursorScreenPoint(&pt)) {
        pt = MessagePoint(message);
    }

    ScrollEvent ev{};
    ev.pt = pt;
    ev.delta = delta;
    ev.horizontal = false;

    ctrl_->InputAutomation().OnScroll(delta);
    ctrl_->IndicatorOverlay().OnScroll(ev);

    bool scrollRenderedByWasm = false;
    bool scrollRouteActive = false;
    if (auto* wasmHost = ctrl_->WasmHost()) {
        scrollRouteActive = wasmHost->Enabled() && wasmHost->IsPluginLoaded();
    }
    if (scrollRouteActive) {
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::Scroll;
        invoke.x = pt.x;
        invoke.y = pt.y;
        invoke.delta = static_cast<int32_t>(delta);
        invoke.flags = ev.horizontal ? wasm::kEventFlagScrollHorizontal : 0x00u;
        invoke.eventTickMs = GetTickCount64();
        bool invokeOk = false;
        TryInvokeAndRenderWasmEvent(invoke, &scrollRenderedByWasm, &invokeOk);
    }
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
    if (!TryReadCursorScreenPoint(&pt)) {
        pt = MessagePoint(message);
    }

    ctrl_->BeginHoldTracking(pt, button);
    ctrl_->InputAutomation().OnButtonDown(pt, button);
    ctrl_->ArmHoldTimer();

    return 0;
}

intptr_t DispatchRouter::OnButtonUp(const DispatchMessage& message) {
    ctrl_->EndHoldTracking();
    ctrl_->CancelPendingHold();

    if (ctrl_->IsVmEffectsSuppressed()) {
        ctrl_->InputAutomation().Reset();
        return 0;
    }

    ScreenPoint pt{};
    if (!TryReadCursorScreenPoint(&pt)) {
        pt.x = 0;
        pt.y = 0;
    }
    ctrl_->InputAutomation().OnButtonUp(pt, static_cast<int>(message.button));

    if (wasmHoldEventActive_) {
        wasm::EventInvokeInput holdEnd{};
        holdEnd.kind = wasm::EventKind::HoldEnd;
        holdEnd.x = pt.x;
        holdEnd.y = pt.y;
        holdEnd.button = wasmHoldButton_;
        holdEnd.eventTickMs = GetTickCount64();
        bool renderedByWasm = false;
        bool invokeOk = false;
        TryInvokeAndRenderWasmEvent(holdEnd, &renderedByWasm, &invokeOk);
        wasmHoldEventActive_ = false;
        wasmHoldButton_ = 0;
    }

    if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
        effect->OnHoldEnd();
    }
    return 0;
}

intptr_t DispatchRouter::OnTimer(const DispatchMessage& message) {
    const UINT_PTR timerId = static_cast<UINT_PTR>(message.timerId);
    if (timerId == AppController::HoverTimerId()) {
        if (ctrl_->IsVmEffectsSuppressed()) {
            return 0;
        }
        ScreenPoint pt{};
        if (ctrl_->TryEnterHover(&pt)) {
            bool hoverRenderedByWasm = false;
            bool hoverRouteActive = false;
            if (auto* wasmHost = ctrl_->WasmHost()) {
                hoverRouteActive = wasmHost->Enabled() && wasmHost->IsPluginLoaded();
            }
            if (hoverRouteActive) {
                wasm::EventInvokeInput invoke{};
                invoke.kind = wasm::EventKind::HoverStart;
                invoke.x = pt.x;
                invoke.y = pt.y;
                invoke.eventTickMs = GetTickCount64();
                bool invokeOk = false;
                TryInvokeAndRenderWasmEvent(invoke, &hoverRenderedByWasm, &invokeOk);
            }
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
            bool holdRouteActive = false;
            bool holdInvokeOk = false;
            if (auto* wasmHost = ctrl_->WasmHost()) {
                holdRouteActive = wasmHost->Enabled() && wasmHost->IsPluginLoaded();
            }
            if (holdRouteActive) {
                wasm::EventInvokeInput invoke{};
                invoke.kind = wasm::EventKind::HoldStart;
                invoke.x = pt.x;
                invoke.y = pt.y;
                invoke.button = ToWasmButtonFromCode(button);
                invoke.holdMs = static_cast<uint32_t>(ctrl_->CurrentHoldDurationMs());
                invoke.eventTickMs = GetTickCount64();
                TryInvokeAndRenderWasmEvent(invoke, &holdRenderedByWasm, &holdInvokeOk);
                wasmHoldEventActive_ = holdInvokeOk;
                wasmHoldButton_ = invoke.button;
            } else {
                wasmHoldEventActive_ = false;
                wasmHoldButton_ = 0;
            }
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
    static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;
    if (timerId == kSelfTestTimerId) {
        const HWND hwnd = ctrl_->DispatchWindowHandle();
        if (hwnd) {
            KillTimer(hwnd, kSelfTestTimerId);
        }
        ClickEvent ev{};
        ScreenPoint cursor{};
        if (!TryReadCursorScreenPoint(&cursor)) {
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
