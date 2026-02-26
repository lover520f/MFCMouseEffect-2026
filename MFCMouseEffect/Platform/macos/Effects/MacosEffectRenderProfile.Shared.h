#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::macos_effect_profile::detail {

inline double ClampDouble(double value, double lo, double hi) {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

inline TrailThrottleProfile ResolveTrailThrottleProfileByType(const std::string& trailType) {
    const std::string type = ToLowerAscii(trailType);
    if (type == "particle") {
        return {10, 3.0};
    }
    if (type == "meteor") {
        return {14, 5.0};
    }
    if (type == "streamer") {
        return {12, 4.0};
    }
    if (type == "electric") {
        return {15, 6.0};
    }
    if (type == "tubes") {
        return {18, 8.0};
    }
    return {};
}

} // namespace mousefx::macos_effect_profile::detail
