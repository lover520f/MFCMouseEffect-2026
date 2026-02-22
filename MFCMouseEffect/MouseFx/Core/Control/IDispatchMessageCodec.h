#pragma once

#include <cstdint>

#include "MouseFx/Core/Control/DispatchMessage.h"

namespace mousefx {

class IDispatchMessageCodec {
public:
    virtual ~IDispatchMessageCodec() = default;

    virtual DispatchMessage Decode(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) const = 0;

    virtual intptr_t DefaultResult(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) const = 0;
};

} // namespace mousefx
