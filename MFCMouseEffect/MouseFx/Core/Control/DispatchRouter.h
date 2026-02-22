#pragma once

#include <cstdint>

#include "MouseFx/Core/Control/DispatchMessage.h"

namespace mousefx {

class AppController;
namespace wasm {
struct EventInvokeInput;
}

// Routes normalized dispatch messages to AppController subsystems.
class DispatchRouter final {
public:
    explicit DispatchRouter(AppController* controller);

    // Main message routing entry point.
    // When outHandled is false, caller should use platform default handling.
    intptr_t Route(const DispatchMessage& message, bool* outHandled);

private:
    bool TryInvokeAndRenderWasmEvent(
        const wasm::EventInvokeInput& input,
        bool* outRenderedByWasm,
        bool* outInvokeOk);
    intptr_t OnClick(const DispatchMessage& message);
    intptr_t OnMove(const DispatchMessage& message);
    intptr_t OnScroll(const DispatchMessage& message);
    intptr_t OnKey(const DispatchMessage& message);
    intptr_t OnButtonDown(const DispatchMessage& message);
    intptr_t OnButtonUp(const DispatchMessage& message);
    intptr_t OnTimer(const DispatchMessage& message);

    AppController* ctrl_ = nullptr;
    bool wasmHoldEventActive_ = false;
    uint8_t wasmHoldButton_ = 0;
};

} // namespace mousefx
