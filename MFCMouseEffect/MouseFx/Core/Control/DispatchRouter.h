#pragma once

#include <cstdint>

#include "MouseFx/Core/Control/AutomationDispatchFeature.h"
#include "MouseFx/Core/Control/DispatchMessage.h"
#include "MouseFx/Core/Control/InputIndicatorDispatchFeature.h"
#include "MouseFx/Core/Control/WasmDispatchFeature.h"

namespace mousefx {

class AppController;

// Routes normalized dispatch messages to AppController subsystems.
class DispatchRouter final {
public:
    explicit DispatchRouter(AppController* controller);

    // Main message routing entry point.
    // When outHandled is false, caller should use platform default handling.
    intptr_t Route(const DispatchMessage& message, bool* outHandled);

private:
    intptr_t OnClick(const DispatchMessage& message);
    intptr_t OnMove(const DispatchMessage& message);
    intptr_t OnScroll(const DispatchMessage& message);
    intptr_t OnKey(const DispatchMessage& message);
    intptr_t OnButtonDown(const DispatchMessage& message);
    intptr_t OnButtonUp(const DispatchMessage& message);
    intptr_t OnTimer(const DispatchMessage& message);

    AppController* ctrl_ = nullptr;
    WasmDispatchFeature wasmFeature_{};
    AutomationDispatchFeature automationFeature_{};
    InputIndicatorDispatchFeature indicatorFeature_{};
};

} // namespace mousefx
