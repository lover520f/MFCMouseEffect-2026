#pragma once
#include "MouseFx/Core/Protocol/InputTypes.h"
#include <gdiplus.h>
#include <deque>
#include "MouseFx/Core/Config/EffectConfig.h" // For Theme/Color

namespace mousefx {

struct TrailPoint {
    ScreenPoint pt{};
    uint64_t addedTime;
};

class ITrailRenderer {
public:
    virtual ~ITrailRenderer() = default;
    virtual void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) = 0;
};

} // namespace mousefx
