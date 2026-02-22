#pragma once

#include "MouseFx/Core/Control/IDispatchMessageCodec.h"

namespace mousefx {

class Win32DispatchMessageCodec final : public IDispatchMessageCodec {
public:
    DispatchMessage Decode(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) const override;

    intptr_t DefaultResult(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) const override;
};

} // namespace mousefx
