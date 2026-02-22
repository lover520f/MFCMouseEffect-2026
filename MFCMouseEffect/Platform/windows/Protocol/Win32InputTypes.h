#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <windows.h>

namespace mousefx {

inline ScreenPoint ToScreenPoint(const POINT& pt) {
    return ScreenPoint{
        static_cast<int32_t>(pt.x),
        static_cast<int32_t>(pt.y),
    };
}

inline POINT ToNativePoint(const ScreenPoint& pt) {
    POINT out{};
    out.x = static_cast<LONG>(pt.x);
    out.y = static_cast<LONG>(pt.y);
    return out;
}

} // namespace mousefx
