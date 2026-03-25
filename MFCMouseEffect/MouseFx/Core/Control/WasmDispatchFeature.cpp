#include "pch.h"

#include "MouseFx/Core/Control/WasmDispatchFeature.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmEventInvokeExecutor.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

#include <algorithm>
#include <array>

namespace mousefx {
namespace {

constexpr const char* kEffectsChannelClick = "click";
constexpr const char* kEffectsChannelTrail = "trail";
constexpr const char* kEffectsChannelScroll = "scroll";
constexpr const char* kEffectsChannelHold = "hold";
constexpr const char* kEffectsChannelHover = "hover";
constexpr const char* kEffectsChannelCursorDecoration = "cursor_decoration";
constexpr std::array<const char*, 6> kEffectsChannels = {
    kEffectsChannelClick,
    kEffectsChannelTrail,
    kEffectsChannelScroll,
    kEffectsChannelHold,
    kEffectsChannelHover,
    kEffectsChannelCursorDecoration,
};

const char* ChannelForEventKind(wasm::EventKind kind) {
    switch (kind) {
    case wasm::EventKind::Click:
        return kEffectsChannelClick;
    case wasm::EventKind::Move:
        return kEffectsChannelTrail;
    case wasm::EventKind::Scroll:
        return kEffectsChannelScroll;
    case wasm::EventKind::HoldStart:
    case wasm::EventKind::HoldUpdate:
    case wasm::EventKind::HoldEnd:
        return kEffectsChannelHold;
    case wasm::EventKind::HoverStart:
    case wasm::EventKind::HoverEnd:
        return kEffectsChannelHover;
    default:
        return kEffectsChannelClick;
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

bool WasmDispatchFeature::IsRouteActive(const AppController& controller, const char* channel) const {
    auto* wasmHost = controller.WasmEffectsHostForChannel(channel ? channel : "");
    return wasmHost && wasmHost->Enabled() && wasmHost->IsPluginLoaded();
}

bool WasmDispatchFeature::TryInvokeAndRender(
    AppController& controller,
    const char* channel,
    const wasm::EventInvokeInput& input,
    bool* outRenderedByWasm,
    bool* outInvokeOk) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (outInvokeOk) {
        *outInvokeOk = false;
    }

    auto* wasmHost = controller.WasmEffectsHostForChannel(channel ? channel : "");
    if (!wasmHost || !wasmHost->Enabled() || !wasmHost->IsPluginLoaded()) {
        return false;
    }
    if (!wasmHost->SupportsInputEvent(input.kind)) {
        return false;
    }

    const wasm::EventDispatchExecutionResult dispatchResult =
        wasm::InvokeEventAndRender(*wasmHost, input, controller.Config());
    const bool renderedByWasm = dispatchResult.render.renderedAny;
    const uint32_t executedTextCommands = dispatchResult.render.executedTextCommands;
    const uint32_t executedImageCommands = dispatchResult.render.executedImageCommands;
    const uint32_t droppedCommands = dispatchResult.render.droppedCommands;
    const bool wasmOk = dispatchResult.invokeOk;

    if (outRenderedByWasm) {
        *outRenderedByWasm = renderedByWasm;
    }
    if (outInvokeOk) {
        *outInvokeOk = wasmOk;
    }

#if MFX_PLATFORM_WINDOWS && defined(_DEBUG)
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
        static_cast<unsigned long>(executedTextCommands),
        static_cast<unsigned long>(executedImageCommands),
        static_cast<unsigned long>(droppedCommands));
    OutputDebugStringW(buffer);
#endif

    return true;
}

bool WasmDispatchFeature::TryInvokeAndRender(
    AppController& controller,
    const wasm::EventInvokeInput& input,
    bool* outRenderedByWasm,
    bool* outInvokeOk) {
    const char* channel = ChannelForEventKind(input.kind);
    return TryInvokeAndRender(controller, channel, input, outRenderedByWasm, outInvokeOk);
}

bool WasmDispatchFeature::RouteClick(AppController& controller, const ClickEvent& ev, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller, kEffectsChannelClick)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::Click;
    invoke.x = ev.pt.x;
    invoke.y = ev.pt.y;
    invoke.button = ToWasmButton(ev.button);
    invoke.eventTickMs = controller.CurrentTickMs();

    return TryInvokeAndRender(controller, invoke, outRenderedByWasm, nullptr);
}

bool WasmDispatchFeature::RouteMove(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    const bool trailActive = IsRouteActive(controller, kEffectsChannelTrail);
    const bool decorationActive = IsRouteActive(controller, kEffectsChannelCursorDecoration);
    if (!trailActive && !decorationActive) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::Move;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.eventTickMs = controller.CurrentTickMs();

    bool renderedAny = false;
    bool routed = false;
    if (trailActive) {
        bool rendered = false;
        routed = TryInvokeAndRender(controller, kEffectsChannelTrail, invoke, &rendered, nullptr) || routed;
        renderedAny = renderedAny || rendered;
    }
    if (decorationActive) {
        bool rendered = false;
        routed =
            TryInvokeAndRender(controller, kEffectsChannelCursorDecoration, invoke, &rendered, nullptr) || routed;
        renderedAny = renderedAny || rendered;
    }
    if (outRenderedByWasm) {
        *outRenderedByWasm = renderedAny;
    }
    return routed;
}

bool WasmDispatchFeature::RouteScroll(AppController& controller, const ScrollEvent& ev, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller, kEffectsChannelScroll)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::Scroll;
    invoke.x = ev.pt.x;
    invoke.y = ev.pt.y;
    invoke.delta = static_cast<int32_t>(ev.delta);
    invoke.flags = ev.horizontal ? wasm::kEventFlagScrollHorizontal : 0x00u;
    invoke.eventTickMs = controller.CurrentTickMs();

    return TryInvokeAndRender(controller, invoke, outRenderedByWasm, nullptr);
}

bool WasmDispatchFeature::RouteHoverStart(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller, kEffectsChannelHover)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::HoverStart;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.eventTickMs = controller.CurrentTickMs();

    return TryInvokeAndRender(controller, invoke, outRenderedByWasm, nullptr);
}

bool WasmDispatchFeature::RouteHoverEnd(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (!IsRouteActive(controller, kEffectsChannelHover)) {
        return false;
    }

    wasm::EventInvokeInput invoke{};
    invoke.kind = wasm::EventKind::HoverEnd;
    invoke.x = pt.x;
    invoke.y = pt.y;
    invoke.eventTickMs = controller.CurrentTickMs();

    return TryInvokeAndRender(controller, invoke, outRenderedByWasm, nullptr);
}

bool WasmDispatchFeature::RouteFrameTick(AppController& controller, bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    bool hasFrameTickLane = false;
    for (const char* channel : kEffectsChannels) {
        auto* host = controller.WasmEffectsHostForChannel(channel);
        if (!host || !host->Enabled() || !host->IsPluginLoaded()) {
            continue;
        }
        if (!host->SupportsFrameTick()) {
            continue;
        }
        hasFrameTickLane = true;
        break;
    }
    if (!hasFrameTickLane) {
        lastFrameTickMs_ = 0;
        return false;
    }

    const uint64_t nowTickMs = controller.CurrentTickMs();
    uint32_t frameDeltaMs = 16;
    if (lastFrameTickMs_ > 0 && nowTickMs >= lastFrameTickMs_) {
        const uint64_t delta = nowTickMs - lastFrameTickMs_;
        frameDeltaMs = static_cast<uint32_t>(std::clamp<uint64_t>(delta, 1, 1000));
    }
    lastFrameTickMs_ = nowTickMs;

    ScreenPoint cursorPt{};
    bool pointerValid = controller.QueryCursorScreenPoint(&cursorPt);
    if (!pointerValid) {
        pointerValid = controller.TryGetLastPointerPoint(&cursorPt);
    } else {
        controller.RememberLastPointerPoint(cursorPt);
    }
    if (!pointerValid) {
        cursorPt.x = 0;
        cursorPt.y = 0;
    }

    wasm::FrameInvokeInput invoke{};
    invoke.cursorX = cursorPt.x;
    invoke.cursorY = cursorPt.y;
    invoke.frameDeltaMs = frameDeltaMs;
    invoke.pointerValid = pointerValid;
    invoke.holdActive = controller.IsHoldButtonDown();
    invoke.frameTickMs = nowTickMs;

    bool renderedAny = false;
    for (const char* channel : kEffectsChannels) {
        auto* wasmHost = controller.WasmEffectsHostForChannel(channel);
        if (!wasmHost || !wasmHost->Enabled() || !wasmHost->IsPluginLoaded()) {
            continue;
        }
        if (!wasmHost->SupportsFrameTick()) {
            continue;
        }
        const wasm::EventDispatchExecutionResult dispatchResult =
            wasm::InvokeFrameAndRender(*wasmHost, invoke, controller.Config());
        renderedAny = renderedAny || dispatchResult.render.renderedAny;
    }
    if (outRenderedByWasm) {
        *outRenderedByWasm = renderedAny;
    }
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
    if (!IsRouteActive(controller, kEffectsChannelHold)) {
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
    const bool routed = TryInvokeAndRender(controller, invoke, outRenderedByWasm, &invokeOk);
    holdEventActive_ = routed && invokeOk;
    holdButton_ = holdEventActive_ ? invoke.button : 0;
    return routed;
}

void WasmDispatchFeature::RouteHoldUpdateIfActive(AppController& controller, const ScreenPoint& pt, uint32_t holdMs) {
    if (!holdEventActive_) {
        return;
    }
    if (!IsRouteActive(controller, kEffectsChannelHold)) {
        ResetHoldState();
        return;
    }
    auto* wasmHost = controller.WasmEffectsHostForChannel(kEffectsChannelHold);
    if (!wasmHost) {
        ResetHoldState();
        return;
    }
    if (!wasmHost->SupportsInputEvent(wasm::EventKind::HoldUpdate)) {
        // Keep hold state alive for a potential HoldEnd route.
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
    const bool routed = TryInvokeAndRender(controller, invoke, &renderedByWasm, &invokeOk);
    if (!routed || !invokeOk) {
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
    TryInvokeAndRender(controller, invoke, &renderedByWasm, nullptr);
    ResetHoldState();
}

void WasmDispatchFeature::ResetHoldState() {
    holdEventActive_ = false;
    holdButton_ = 0;
}

} // namespace mousefx
