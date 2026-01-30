#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <deque>
#include "MouseFx/Core/EffectConfig.h" // For Theme/Color

namespace mousefx {

struct TrailPoint {
    POINT pt;
    uint64_t addedTime;
};

class ITrailRenderer {
public:
    virtual ~ITrailRenderer() = default;
    virtual void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) = 0;
};

} // namespace mousefx
