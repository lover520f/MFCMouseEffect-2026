#pragma once

#include <vector>

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

struct InputIndicatorAnchor final {
    ScreenPoint topLeft{};
    ScreenPoint center{};
    int sizePx = 0;
    int durationMs = 0;
};

std::vector<InputIndicatorAnchor> ResolveInputIndicatorAnchors(
    const InputIndicatorConfig& cfg,
    const ScreenPoint& anchorPt);

} // namespace mousefx
