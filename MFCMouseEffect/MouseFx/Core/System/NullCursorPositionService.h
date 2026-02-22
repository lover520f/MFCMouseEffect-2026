#pragma once

#include "MouseFx/Core/System/ICursorPositionService.h"

namespace mousefx {

class NullCursorPositionService final : public ICursorPositionService {
public:
    bool TryGetCursorScreenPoint(ScreenPoint* /*outPt*/) const override {
        return false;
    }
};

} // namespace mousefx
