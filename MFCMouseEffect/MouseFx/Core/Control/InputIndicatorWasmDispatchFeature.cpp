#include "pch.h"

#include "MouseFx/Core/Control/InputIndicatorWasmDispatchFeature.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Overlay/InputIndicatorPositionResolver.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmEventInvokeExecutor.h"
#include "Platform/PlatformTarget.h"

#include <algorithm>

#if MFX_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace mousefx {
namespace {

uint64_t ResolveClickStreakTimeoutMs() {
#if MFX_PLATFORM_WINDOWS
    return static_cast<uint64_t>(std::max<DWORD>(::GetDoubleClickTime(), 240u) + 120u);
#else
    return 620;
#endif
}

uint64_t ResolveScrollStreakTimeoutMs() {
#if MFX_PLATFORM_WINDOWS
    return static_cast<uint64_t>(::GetDoubleClickTime());
#else
    return 500;
#endif
}

uint64_t ResolveKeyStreakTimeoutMs() {
#if MFX_PLATFORM_WINDOWS
    return static_cast<uint64_t>(::GetDoubleClickTime()) * 2;
#else
    return 1000;
#endif
}

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

uint8_t ToWasmModifierMask(const KeyEvent& ev) {
    return static_cast<uint8_t>(PackInputIndicatorModifierMask(ev));
}

InputIndicatorKeyLabelOptions BuildKeyLabelOptions() {
    InputIndicatorKeyLabelOptions options{};
#if MFX_PLATFORM_MACOS
    options.metaModifierLabel = "Cmd";
#endif
    return options;
}

bool TryInvokeIndicatorEvent(
    AppController& controller,
    const wasm::EventInvokeInput& input,
    InputIndicatorWasmRouteTrace* outTrace,
    bool* outRenderedByWasm) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }

    auto* wasmHost = controller.WasmIndicatorHost();
    if (outTrace) {
        outTrace->hostPresent = (wasmHost != nullptr);
    }
    if (!wasmHost) {
        return false;
    }
    if (outTrace) {
        outTrace->hostEnabled = wasmHost->Enabled();
        outTrace->pluginLoaded = wasmHost->IsPluginLoaded();
    }
    if (!wasmHost->Enabled() || !wasmHost->IsPluginLoaded()) {
        return false;
    }
    const bool eventSupported = wasmHost->SupportsInputEvent(input.kind);
    if (outTrace) {
        outTrace->eventSupported = eventSupported;
    }
    if (!eventSupported) {
        return false;
    }

    const wasm::EventDispatchExecutionResult dispatchResult =
        wasm::InvokeEventAndRender(*wasmHost, input, controller.Config());
    if (outTrace) {
        outTrace->invokeAttempted = true;
        outTrace->renderedAny = outTrace->renderedAny || dispatchResult.render.renderedAny;
    }
    if (outRenderedByWasm) {
        *outRenderedByWasm = dispatchResult.render.renderedAny;
    }
    return true;
}

} // namespace

bool InputIndicatorWasmDispatchFeature::RouteClick(
    AppController& controller,
    const ClickEvent& ev,
    bool* outRenderedByWasm,
    InputIndicatorWasmRouteTrace* outTrace) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (outTrace) {
        *outTrace = InputIndicatorWasmRouteTrace{};
        outTrace->routeAttempted = true;
    }
    const auto anchors = ResolveInputIndicatorAnchors(controller.Config().inputIndicator, ev.pt);
    if (anchors.empty()) {
        return false;
    }
    if (outTrace) {
        outTrace->anchorsResolved = true;
    }

    const uint64_t now = controller.CurrentTickMs();
    const uint64_t timeout = ResolveClickStreakTimeoutMs();
    const int streak = AdvanceInputIndicatorClickStreak(&mouseStreakState_, ev.button, now, timeout, 3);
    const std::string label = BuildInputIndicatorClickLabel(ev.button, streak);

    bool anyRendered = false;
    bool anyInvoked = false;
    for (const auto& anchor : anchors) {
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::IndicatorClick;
        invoke.x = anchor.center.x;
        invoke.y = anchor.center.y;
        invoke.button = ToWasmButton(ev.button);
        invoke.eventTickMs = now;
        invoke.indicatorMetrics.sizePx = static_cast<uint16_t>(anchor.sizePx);
        invoke.indicatorMetrics.durationMs = static_cast<uint16_t>(anchor.durationMs);
        invoke.hasIndicatorMetrics = true;
        invoke.indicatorContext.primaryCode = static_cast<uint32_t>(invoke.button);
        invoke.indicatorContext.streak = static_cast<uint16_t>(streak);
        invoke.hasIndicatorContext = true;
        invoke.dynamicTextLabelUtf8 = label;

        bool rendered = false;
        const bool invoked = TryInvokeIndicatorEvent(controller, invoke, outTrace, &rendered);
        anyInvoked = anyInvoked || invoked;
        anyRendered = anyRendered || rendered;
    }

    if (outRenderedByWasm) {
        *outRenderedByWasm = anyRendered;
    }
    return anyInvoked;
}

bool InputIndicatorWasmDispatchFeature::RouteScroll(
    AppController& controller,
    const ScrollEvent& ev,
    bool* outRenderedByWasm,
    InputIndicatorWasmRouteTrace* outTrace) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (outTrace) {
        *outTrace = InputIndicatorWasmRouteTrace{};
        outTrace->routeAttempted = true;
    }
    if (ev.delta == 0) {
        return false;
    }

    const auto anchors = ResolveInputIndicatorAnchors(controller.Config().inputIndicator, ev.pt);
    if (anchors.empty()) {
        return false;
    }
    if (outTrace) {
        outTrace->anchorsResolved = true;
    }

    mouseStreakState_.clickStreak = 0;
    const uint64_t now = controller.CurrentTickMs();
    const uint64_t timeout = ResolveScrollStreakTimeoutMs();
    const int streak = AdvanceInputIndicatorScrollStreak(&mouseStreakState_, ev.delta, now, timeout);
    const std::string label = BuildInputIndicatorScrollLabel(ev.delta, streak);

    bool anyRendered = false;
    bool anyInvoked = false;
    for (const auto& anchor : anchors) {
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::IndicatorScroll;
        invoke.x = anchor.center.x;
        invoke.y = anchor.center.y;
        invoke.delta = static_cast<int32_t>(ev.delta);
        invoke.flags = ev.horizontal ? wasm::kEventFlagScrollHorizontal : 0x00u;
        invoke.eventTickMs = now;
        invoke.indicatorMetrics.sizePx = static_cast<uint16_t>(anchor.sizePx);
        invoke.indicatorMetrics.durationMs = static_cast<uint16_t>(anchor.durationMs);
        invoke.hasIndicatorMetrics = true;
        const int64_t deltaMagnitude = ev.delta < 0 ? -static_cast<int64_t>(ev.delta) : static_cast<int64_t>(ev.delta);
        invoke.indicatorContext.primaryCode = static_cast<uint32_t>(std::min<int64_t>(deltaMagnitude, 0xFFFFFFFFll));
        invoke.indicatorContext.streak = static_cast<uint16_t>(streak);
        invoke.hasIndicatorContext = true;
        invoke.dynamicTextLabelUtf8 = label;

        bool rendered = false;
        const bool invoked = TryInvokeIndicatorEvent(controller, invoke, outTrace, &rendered);
        anyInvoked = anyInvoked || invoked;
        anyRendered = anyRendered || rendered;
    }

    if (outRenderedByWasm) {
        *outRenderedByWasm = anyRendered;
    }
    return anyInvoked;
}

bool InputIndicatorWasmDispatchFeature::RouteKey(
    AppController& controller,
    const KeyEvent& ev,
    bool* outRenderedByWasm,
    InputIndicatorWasmRouteTrace* outTrace) {
    if (outRenderedByWasm) {
        *outRenderedByWasm = false;
    }
    if (outTrace) {
        *outTrace = InputIndicatorWasmRouteTrace{};
        outTrace->routeAttempted = true;
    }

    const auto anchors = ResolveInputIndicatorAnchors(controller.Config().inputIndicator, ev.pt);
    if (anchors.empty()) {
        return false;
    }
    if (outTrace) {
        outTrace->anchorsResolved = true;
    }

    const uint64_t now = controller.CurrentTickMs();
    const uint64_t timeout = ResolveKeyStreakTimeoutMs();
    const int streak = AdvanceInputIndicatorKeyStreak(&keyStreakState_, ev, now, timeout);
    InputIndicatorKeyLabelOptions options = BuildKeyLabelOptions();
    std::string label = BuildInputIndicatorKeyLabel(ev, options);
    label = AppendInputIndicatorKeyStreak(std::move(label), streak);

    bool anyRendered = false;
    bool anyInvoked = false;
    for (const auto& anchor : anchors) {
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::IndicatorKey;
        invoke.x = anchor.center.x;
        invoke.y = anchor.center.y;
        invoke.eventTickMs = now;
        invoke.indicatorMetrics.sizePx = static_cast<uint16_t>(anchor.sizePx);
        invoke.indicatorMetrics.durationMs = static_cast<uint16_t>(anchor.durationMs);
        invoke.hasIndicatorMetrics = true;
        invoke.indicatorContext.primaryCode = ev.vkCode;
        invoke.indicatorContext.streak = static_cast<uint16_t>(streak);
        invoke.indicatorContext.modifierMask = ToWasmModifierMask(ev);
        invoke.indicatorContext.detailFlags = ev.systemKey ? wasm::kIndicatorDetailFlagKeySystem : 0u;
        invoke.hasIndicatorContext = true;
        invoke.dynamicTextLabelUtf8 = label;

        bool rendered = false;
        const bool invoked = TryInvokeIndicatorEvent(controller, invoke, outTrace, &rendered);
        anyInvoked = anyInvoked || invoked;
        anyRendered = anyRendered || rendered;
    }

    if (outRenderedByWasm) {
        *outRenderedByWasm = anyRendered;
    }
    return anyInvoked;
}

} // namespace mousefx
