// DispatchRouter.cpp -- Win32 message routing extracted from AppController

#include "pch.h"
#include "DispatchRouter.h"
#include "AppController.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
#include "MouseFx/Core/Protocol/InputTypesWin32.h"
#include "MouseFx/Core/Wasm/WasmClickCommandExecutor.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"

#include <windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM
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

LRESULT DispatchRouter::Route(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ctrl_->OnDispatchActivity(msg, wParam);

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
                *out = ctrl_->Config();
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

LRESULT DispatchRouter::OnClick(HWND /*hwnd*/, LPARAM lParam) {
    auto* ev = reinterpret_cast<ClickEvent*>(lParam);
    if (ctrl_->IsVmEffectsSuppressed()) {
        if (ev) delete ev;
        return 0;
    }
    if (ctrl_->ConsumeIgnoreNextClick()) {
        if (ev) delete ev;
        return 0;  // Suppress click after a long hold
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

LRESULT DispatchRouter::OnMove(HWND /*hwnd*/, WPARAM wParam, LPARAM lParam) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        return 0;
    }
    POINT pt{};
    if (!ctrl_->ConsumeLatestMove(&pt)) {
        pt.x = static_cast<LONG>(wParam);
        pt.y = static_cast<LONG>(lParam);
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
            effect->OnMouseMove(ToScreenPoint(pt));
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
        effect->OnHoldUpdate(ToScreenPoint(pt), ctrl_->CurrentHoldDurationMs());
    }
    return 0;
}

LRESULT DispatchRouter::OnScroll(HWND /*hwnd*/, WPARAM wParam, LPARAM lParam) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        return 0;
    }
    short delta = static_cast<short>(wParam);
    POINT pt{};
    if (!GetCursorPos(&pt)) {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    }
    ScrollEvent ev{};
    ev.pt = ToScreenPoint(pt);
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

LRESULT DispatchRouter::OnKey(HWND /*hwnd*/, LPARAM lParam) {
    auto* ev = reinterpret_cast<KeyEvent*>(lParam);
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

LRESULT DispatchRouter::OnButtonDown(HWND /*hwnd*/, WPARAM wParam, LPARAM lParam) {
    if (ctrl_->IsVmEffectsSuppressed()) {
        ctrl_->ClearPendingHold();
        return 0;
    }
    int button = static_cast<int>(wParam);
    POINT pt{};
    if (!GetCursorPos(&pt)) {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    }

    ctrl_->BeginHoldTracking(pt, button);
    ctrl_->InputAutomation().OnButtonDown(pt, button);
    ctrl_->ArmHoldTimer();

    return 0;
}

LRESULT DispatchRouter::OnButtonUp(HWND /*hwnd*/, WPARAM wParam, LPARAM /*lParam*/) {
    ctrl_->EndHoldTracking();
    ctrl_->CancelPendingHold();

    if (ctrl_->IsVmEffectsSuppressed()) {
        ctrl_->InputAutomation().Reset();
        return 0;
    }

    POINT pt{};
    if (!GetCursorPos(&pt)) {
        pt.x = 0;
        pt.y = 0;
    }
    ctrl_->InputAutomation().OnButtonUp(pt, static_cast<int>(wParam));

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

LRESULT DispatchRouter::OnTimer(HWND hwnd, WPARAM wParam) {
    if (wParam == AppController::HoverTimerId()) {
        if (ctrl_->IsVmEffectsSuppressed()) {
            return 0;
        }
        POINT pt{};
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
                    effect->OnHoverStart(ToScreenPoint(pt));
                }
            }
        }
        return 0;
    }

    if (wParam == AppController::HoldTimerId()) {
        KillTimer(hwnd, AppController::HoldTimerId());
        if (ctrl_->IsVmEffectsSuppressed()) {
            ctrl_->ClearPendingHold();
            return 0;
        }
        POINT pt{};
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
                    effect->OnHoldStart(ToScreenPoint(pt), button);
                }
            }
            ctrl_->MarkIgnoreNextClick();  // Timer fired = Hold triggered = ignore next click.
        }
        return 0;
    }

#ifdef _DEBUG
    static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;
    if (wParam == kSelfTestTimerId) {
        KillTimer(ctrl_->DispatchWindowHandle(), kSelfTestTimerId);
        ClickEvent ev{};
        POINT cursor{};
        GetCursorPos(&cursor);
        ev.pt = ToScreenPoint(cursor);
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
