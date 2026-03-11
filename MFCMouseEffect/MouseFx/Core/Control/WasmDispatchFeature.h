#pragma once

#include <cstdint>

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class AppController;
struct ScrollEvent;

namespace wasm {
struct EventInvokeInput;
}

// Encapsulates WASM event routing and hold-stream state for dispatch path.
class WasmDispatchFeature final {
public:
    bool RouteClick(AppController& controller, const ClickEvent& ev, bool* outRenderedByWasm);
    bool RouteMove(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm);
    bool RouteScroll(AppController& controller, const ScrollEvent& ev, bool* outRenderedByWasm);
    bool RouteHoverStart(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm);
    bool RouteHoverEnd(AppController& controller, const ScreenPoint& pt, bool* outRenderedByWasm);
    bool RouteFrameTick(AppController& controller, bool* outRenderedByWasm);
    bool RouteHoldStart(
        AppController& controller,
        const ScreenPoint& pt,
        int button,
        uint32_t holdMs,
        bool* outRenderedByWasm);
    void RouteHoldUpdateIfActive(AppController& controller, const ScreenPoint& pt, uint32_t holdMs);
    void RouteHoldEndIfActive(AppController& controller, const ScreenPoint& pt);
    void ResetHoldState();

private:
    bool IsRouteActive(const AppController& controller, const char* channel) const;
    bool TryInvokeAndRender(
        AppController& controller,
        const wasm::EventInvokeInput& input,
        bool* outRenderedByWasm,
        bool* outInvokeOk);
    static uint8_t ToWasmButton(MouseButton button);
    static uint8_t ToWasmButtonFromCode(int button);

private:
    bool holdEventActive_ = false;
    uint8_t holdButton_ = 0;
    uint64_t lastFrameTickMs_ = 0;
};

} // namespace mousefx
