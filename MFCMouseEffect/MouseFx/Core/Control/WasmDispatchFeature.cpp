#include "pch.h"

#include "MouseFx/Core/Control/WasmDispatchFeature.h"

#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmClickCommandExecutor.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

namespace mousefx {

namespace {

void ResetHoldState(bool* holdEventActive, uint8_t* holdButton) {
    if (holdEventActive) {
        *holdEventActive = false;
    }
    if (holdButton) {
        *holdButton = 0;
    }
}

} // namespace

uint8_t WasmDispatchFeature::ToWasmButton(MouseButton button) {
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

uint8_t WasmDispatchFeature::ToWasmButtonFromCode(int button) {
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

bool WasmDispatchFeature::IsRouteActive(const AppController& controller) const {
    auto* wasmHost = controller.WasmHost();
    return wasmHost && wasmHost->Enabled() && wasmHost->IsPluginLoaded();
}

bool WasmDispatchFeature::TryInvokeAndRender(
    AppController& controller,
    const wasm::EventInvokeInput& input,
    bool* outRenderedByWasm,
    bool* outInvokeOk) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (outInvokeOk) {
        *outInvokeOk = false;
    }

    auto* wasmHost = controller.WasmHost();
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
            controller.Config(),
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

bool WasmDispatchFeature::RouteClick(AppController& controller, const ClickEvent& ev, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::Click;
    invoke.x = ev.pt.x;
    invoke.y = ev.pt.y;
    invoke.button = ToWasmButton(ev.button);
    invoke.eventTickMs = controller.CurrentTickMs();

    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, outRenderedByWasm, &invokeOk);
    return true;
}

bool WasmDispatchFeature::RouteMove(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::Move;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, outRenderedByWasm, &invokeOk);
    return true;
}

bool WasmDispatchFeature::RouteScroll(AppController& controller, const ScrollEvent& ev, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::Scroll;
    invoke.x = ev.pt.x;
    invoke.y = ev.pt.y;
    invoke.delta = static_cast<int32_t>(ev.delta);
    invoke.flags = ev.horizontal ? wasm::kEventFlagScrollHorizontal : 0x00u;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, outRenderedByWasm, &invokeOk);
    return true;
}

bool WasmDispatchFeature::RouteHoverStart(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::HoverStart;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, outRenderedByWasm, &invokeOk);
    return true;
}

bool WasmDispatchFeature::RouteHoldStart(
    AppController& controller,
    const ScreenPoint& pt,
    int button,
    uint32_t holdMs,
    bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller)) {
        ResetHoldState();
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::HoldStart;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.button = ToWasmButtonFromCode(button);
    invoke.holdMs = holdMs;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, outRenderedByWasm, &invokeOk);
    holdEventActive_ = invokeOk;
    holdButton_ = invoke.button;
    return true;
}

void WasmDispatchFeature::RouteHoldUpdateIfActive(AppController& controller, const ScreenPoint& pt, uint32_t holdMs) {
    if (!holdEventActive_) {
        return;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::HoldUpdate;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.button = holdButton_;
    invoke.holdMs = holdMs;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool renderedByWasm = false;
    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, &renderedByWasm, &invokeOk);
    if (!invokeOk) {
        ResetHoldState();
    }
}

void WasmDispatchFeature::RouteHoldEndIfActive(AppController& controller, const ScreenPoint& pt) {
    if (!holdEventActive_) {
        return;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::HoldEnd;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.button = holdButton_;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool renderedByWasm = false;
    bool invokeOk = false;
    TryInvokeAndRender(controller, invoke, &renderedByWasm, &invokeOk);
    ResetHoldState();
}

void WasmDispatchFeature::ResetHoldState() {
    ::mousefx::ResetHoldState(&holdEventActive_, &holdButton_);
}

} // namespace mousefx
