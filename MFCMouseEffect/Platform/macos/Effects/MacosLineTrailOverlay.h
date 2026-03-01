#pragma once

#include <cstdint>

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/Config/EffectConfig.h"

namespace mousefx::macos_line_trail {

struct LineTrailConfig final {
    int durationMs = 300;
    float lineWidth = 4.0f;
    uint32_t strokeArgb = 0xFFFFFFFFu;
    IdleFadeParams idleFade{};
};

struct LineTrailRuntimeSnapshot final {
    bool active = false;
    int pointCount = 0;
};

void UpdateLineTrail(const ScreenPoint& overlayPt, const LineTrailConfig& config);
void ResetLineTrail();
LineTrailRuntimeSnapshot ReadLineTrailRuntimeSnapshot();

} // namespace mousefx::macos_line_trail
