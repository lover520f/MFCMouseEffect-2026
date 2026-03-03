#pragma once

#include <cstdint>

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/Config/EffectConfig.h"

namespace mousefx::macos_line_trail {

enum class LineTrailStyleKind : int32_t {
    Line = 0,
    Streamer = 1,
    Electric = 2,
    Meteor = 3,
    Tubes = 4,
    Particle = 5,
};

struct LineTrailConfig final {
    int durationMs = 300;
    float lineWidth = 4.0f;
    uint32_t strokeArgb = 0xFFFFFFFFu;
    uint32_t fillArgb = 0x66FFFFFFu;
    double intensity = 0.0;
    LineTrailStyleKind style = LineTrailStyleKind::Line;
    bool chromatic = false;
    float streamerGlowWidthScale = 1.8f;
    float streamerCoreWidthScale = 0.55f;
    float streamerHeadPower = 1.6f;
    float electricAmplitudeScale = 1.0f;
    float electricForkChance = 0.10f;
    float meteorSparkRateScale = 1.0f;
    float meteorSparkSpeedScale = 1.0f;
    IdleFadeParams idleFade{};
};

struct LineTrailRuntimeSnapshot final {
    bool active = false;
    int pointCount = 0;
    double lineWidthPx = 0.0;
};

void UpdateLineTrail(const ScreenPoint& overlayPt, const LineTrailConfig& config);
void ResetLineTrail();
LineTrailRuntimeSnapshot ReadLineTrailRuntimeSnapshot();

} // namespace mousefx::macos_line_trail
