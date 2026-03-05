#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>
#include <vector>

namespace mousefx::macos_trail_pulse {

struct TrailPulseEmissionPlannerConfig final {
    double teleportSkipDistancePx = 1600.0;
    int maxSegments = 20;
};

struct TrailPulseEmissionPlan final {
    bool dropAsTeleport = false;
    std::vector<ScreenPoint> segmentPoints{};
};

TrailPulseEmissionPlannerConfig ResolveTrailPulseEmissionPlannerConfig();

TrailPulseEmissionPlan BuildTrailPulseEmissionPlan(
    const ScreenPoint& from,
    const ScreenPoint& to,
    const std::string& normalizedType,
    double throttleMinDistancePx,
    const TrailPulseEmissionPlannerConfig& config);

} // namespace mousefx::macos_trail_pulse
