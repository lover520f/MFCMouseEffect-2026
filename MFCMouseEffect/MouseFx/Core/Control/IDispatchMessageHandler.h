#pragma once

#include <cstdint>

namespace mousefx {

// Cross-platform dispatch callback receiver.
class IDispatchMessageHandler {
public:
    virtual ~IDispatchMessageHandler() = default;

    virtual intptr_t OnDispatchMessage(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) = 0;
};

} // namespace mousefx
