#pragma once

#include "MouseFx/Core/System/ICursorPositionService.h"

namespace mousefx {

class Win32CursorPositionService final : public ICursorPositionService {
public:
    bool TryGetCursorScreenPoint(ScreenPoint* outPt) const override;
};

} // namespace mousefx
