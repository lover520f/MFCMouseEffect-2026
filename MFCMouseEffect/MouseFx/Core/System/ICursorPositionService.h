#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

// Cross-platform cursor position query abstraction.
class ICursorPositionService {
public:
    virtual ~ICursorPositionService() = default;

    virtual bool TryGetCursorScreenPoint(ScreenPoint* outPt) const = 0;
};

} // namespace mousefx
