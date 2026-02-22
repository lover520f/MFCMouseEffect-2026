#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

bool TryGetCursorScreenPoint(ScreenPoint* outPt);

inline bool TryGetCursorScreenPoint(ScreenPoint& outPt) {
    return TryGetCursorScreenPoint(&outPt);
}

} // namespace mousefx
