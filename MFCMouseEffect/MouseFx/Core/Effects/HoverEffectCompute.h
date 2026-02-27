#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

namespace mousefx {

struct HoverEffectColorProfile {
    uint32_t glowFillArgb = 0;
    uint32_t glowStrokeArgb = 0;
    uint32_t tubesStrokeArgb = 0;
};

struct HoverEffectProfile {
    int sizePx = 172;
    double breatheDurationSec = 0.85;
    double spinDurationSec = 1.6;
    double baseOpacity = 0.9;
    double glowSizeScale = 0.96;
    double tubesSizeScale = 1.08;
    double glowBreatheScale = 0.92;
    double tubesBreatheScale = 1.15;
    double tubesSpinScale = 0.82;
    HoverEffectColorProfile colors{};
};

struct HoverEffectRenderCommand {
    ScreenPoint overlayPoint{};
    std::string normalizedType = "glow";
    bool tubesMode = false;
    int sizePx = 172;
    double breatheDurationSec = 0.85;
    double tubesSpinDurationSec = 1.6;
    double baseOpacity = 0.9;
    uint32_t glowFillArgb = 0;
    uint32_t glowStrokeArgb = 0;
    uint32_t tubesStrokeArgb = 0;
};

std::string NormalizeHoverEffectType(const std::string& effectType);
HoverEffectRenderCommand ComputeHoverEffectRenderCommand(
    const ScreenPoint& overlayPoint,
    const std::string& effectType,
    const HoverEffectProfile& profile);

} // namespace mousefx
