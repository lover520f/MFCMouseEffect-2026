// DispatchRouter.cpp -- Win32 message routing extracted from AppController

#include "pch.h"
#include "DispatchRouter.h"
#include "AppController.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
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

} // namespace

DispatchRouter::DispatchRouter(AppController* controller)
    : ctrl_(controller) {}

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
        if (auto* wasmHost = ctrl_->WasmHost()) {
            if (wasmHost->Enabled() && wasmHost->IsPluginLoaded()) {
                wasm::ClickInvokeInput invoke{};
                invoke.x = ev->pt.x;
                invoke.y = ev->pt.y;
                invoke.button = ToWasmButton(ev->button);
                invoke.eventTickMs = GetTickCount64();
                std::vector<uint8_t> commandBuffer;
                const bool wasmOk = wasmHost->InvokeClick(invoke, &commandBuffer);
                wasm::CommandExecutionResult execResult{};
                if (wasmOk && !commandBuffer.empty()) {
                    execResult = wasm::WasmClickCommandExecutor::Execute(
                        commandBuffer.data(),
                        commandBuffer.size(),
                        ctrl_->Config());
                    renderedByWasm = execResult.renderedAny;
                }
                wasmHost->RecordRenderExecution(
                    renderedByWasm,
                    execResult.executedTextCommands,
                    execResult.executedImageCommands,
                    execResult.droppedCommands,
                    execResult.lastError);
#ifdef _DEBUG
                const wasm::HostDiagnostics& diag = wasmHost->Diagnostics();
                wchar_t buffer[256]{};
                wsprintfW(
                    buffer,
                    L"MouseFx: wasm_click ok=%d bytes=%lu commands=%lu parse=%hs err=%hs rendered=%d text=%lu image=%lu drop=%lu\n",
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
#else
                (void)wasmOk;
#endif
            }
        }
        ctrl_->InputAutomation().OnClick(*ev);
        ctrl_->IndicatorOverlay().OnClick(*ev);
        ctrl_->LogDebugClick(*ev);
        if (!renderedByWasm) {
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
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Trail)) {
        effect->OnMouseMove(pt);
    }
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
        effect->OnHoldUpdate(pt, ctrl_->CurrentHoldDurationMs());
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
    ev.pt = pt;
    ev.delta = delta;
    ev.horizontal = false;
    ctrl_->InputAutomation().OnScroll(delta);
    ctrl_->IndicatorOverlay().OnScroll(ev);
    if (auto* effect = ctrl_->GetEffect(EffectCategory::Scroll)) {
        effect->OnScroll(ev);
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

LRESULT DispatchRouter::OnButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam) {
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
    SetTimer(hwnd, AppController::HoldTimerId(), AppController::HoldDelayMs(), nullptr);

    return 0;
}

LRESULT DispatchRouter::OnButtonUp(HWND hwnd, WPARAM wParam, LPARAM /*lParam*/) {
    ctrl_->EndHoldTracking();
    ctrl_->CancelPendingHold(hwnd);

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
            if (auto* effect = ctrl_->GetEffect(EffectCategory::Hover)) {
                effect->OnHoverStart(pt);
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
            if (auto* effect = ctrl_->GetEffect(EffectCategory::Hold)) {
                effect->OnHoldStart(pt, button);
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
