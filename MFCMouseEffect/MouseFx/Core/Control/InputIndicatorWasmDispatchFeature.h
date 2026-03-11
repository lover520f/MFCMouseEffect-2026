#pragma once

#include <cstdint>

#include "MouseFx/Core/Overlay/InputIndicatorLabelFormatter.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class AppController;
struct ScrollEvent;

struct InputIndicatorWasmRouteTrace {
    bool routeAttempted{false};
    bool anchorsResolved{false};
    bool hostPresent{false};
    bool hostEnabled{false};
    bool pluginLoaded{false};
    bool eventSupported{false};
    bool invokeAttempted{false};
    bool renderedAny{false};
};

class InputIndicatorWasmDispatchFeature final {
public:
    bool RouteClick(
        AppController& controller,
        const ClickEvent& ev,
        bool* outRenderedByWasm,
        InputIndicatorWasmRouteTrace* outTrace);
    bool RouteScroll(
        AppController& controller,
        const ScrollEvent& ev,
        bool* outRenderedByWasm,
        InputIndicatorWasmRouteTrace* outTrace);
    bool RouteKey(
        AppController& controller,
        const KeyEvent& ev,
        bool* outRenderedByWasm,
        InputIndicatorWasmRouteTrace* outTrace);

private:
    InputIndicatorMouseStreakState mouseStreakState_{};
    InputIndicatorKeyStreakState keyStreakState_{};
};

} // namespace mousefx
