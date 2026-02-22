#pragma once

#include "MouseFx/Core/Control/IDispatchMessageCodec.h"

namespace mousefx {

class NullDispatchMessageCodec final : public IDispatchMessageCodec {
public:
    DispatchMessage Decode(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) const override {
        DispatchMessage out{};
        out.kind = DispatchMessageKind::Unknown;
        out.sourceHandle = sourceHandle;
        out.nativeMsg = msg;
        out.nativeWParam = wParam;
        out.nativeLParam = lParam;
        return out;
    }

    intptr_t DefaultResult(
        uintptr_t /*sourceHandle*/,
        uint32_t /*msg*/,
        uintptr_t /*wParam*/,
        intptr_t /*lParam*/) const override {
        return 0;
    }
};

} // namespace mousefx
