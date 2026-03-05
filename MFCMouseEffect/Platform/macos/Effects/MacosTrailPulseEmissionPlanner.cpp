#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseEmissionPlanner.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace mousefx::macos_trail_pulse {
namespace {

constexpr const char* kTeleportDistanceEnv = "MFX_MACOS_TRAIL_TELEPORT_SKIP_DISTANCE_PX";
constexpr const char* kMaxSegmentsEnv = "MFX_MACOS_TRAIL_MAX_SEGMENTS";

double ClampDouble(double value, double lo, double hi) {
    return std::clamp(value, lo, hi);
}

int ClampIntValue(int value, int lo, int hi) {
    return std::clamp(value, lo, hi);
}

bool TryParseEnvDouble(const char* key, double* outValue) {
    if (outValue == nullptr) {
        return false;
    }
    const char* raw = std::getenv(key);
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }
    char* end = nullptr;
    const double parsed = std::strtod(raw, &end);
    if (end == raw || (end != nullptr && *end != '\0') || !std::isfinite(parsed)) {
        return false;
    }
    *outValue = parsed;
    return true;
}

bool TryParseEnvInt(const char* key, int* outValue) {
    if (outValue == nullptr) {
        return false;
    }
    const char* raw = std::getenv(key);
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }
    char* end = nullptr;
    const long parsed = std::strtol(raw, &end, 10);
    if (end == raw || (end != nullptr && *end != '\0')) {
        return false;
    }
    if (parsed < static_cast<long>(std::numeric_limits<int>::min()) ||
        parsed > static_cast<long>(std::numeric_limits<int>::max())) {
        return false;
    }
    *outValue = static_cast<int>(parsed);
    return true;
}

double ResolvePreferredSegmentStep(const std::string& normalizedType, double throttleMinDistancePx) {
    const std::string type = ToLowerAscii(normalizedType);
    const double throttleBase = ClampDouble(throttleMinDistancePx, 1.0, 32.0);
    if (type == "streamer") {
        return std::max(6.0, throttleBase * 0.9);
    }
    if (type == "electric") {
        return std::max(5.0, throttleBase * 0.8);
    }
    if (type == "meteor") {
        return std::max(6.0, throttleBase * 0.9);
    }
    if (type == "tubes") {
        return std::max(7.0, throttleBase * 1.0);
    }
    if (type == "particle") {
        return std::max(4.0, throttleBase * 0.6);
    }
    return std::max(5.0, throttleBase * 0.7);
}

void AppendUniquePoint(std::vector<ScreenPoint>* points, const ScreenPoint& point) {
    if (points == nullptr) {
        return;
    }
    if (!points->empty() && points->back().x == point.x && points->back().y == point.y) {
        return;
    }
    points->push_back(point);
}

} // namespace

TrailPulseEmissionPlannerConfig ResolveTrailPulseEmissionPlannerConfig() {
    TrailPulseEmissionPlannerConfig config{};
    double parsedDistance = 0.0;
    if (TryParseEnvDouble(kTeleportDistanceEnv, &parsedDistance)) {
        config.teleportSkipDistancePx = ClampDouble(parsedDistance, 400.0, 8000.0);
    }
    int parsedSegments = 0;
    if (TryParseEnvInt(kMaxSegmentsEnv, &parsedSegments)) {
        config.maxSegments = ClampIntValue(parsedSegments, 1, 64);
    }
    return config;
}

TrailPulseEmissionPlan BuildTrailPulseEmissionPlan(
    const ScreenPoint& from,
    const ScreenPoint& to,
    const std::string& normalizedType,
    double throttleMinDistancePx,
    const TrailPulseEmissionPlannerConfig& config) {
    TrailPulseEmissionPlan plan{};

    const double totalDx = static_cast<double>(to.x - from.x);
    const double totalDy = static_cast<double>(to.y - from.y);
    const double totalDistance = std::sqrt(totalDx * totalDx + totalDy * totalDy);
    if (totalDistance < 0.0001) {
        return plan;
    }

    if (totalDistance > std::max(400.0, config.teleportSkipDistancePx)) {
        plan.dropAsTeleport = true;
        return plan;
    }

    const double step = ResolvePreferredSegmentStep(normalizedType, throttleMinDistancePx);
    const int rawSegments = static_cast<int>(std::ceil(totalDistance / std::max(1.0, step)));
    const int segmentCount = ClampIntValue(rawSegments, 1, std::max(1, config.maxSegments));

    plan.segmentPoints.reserve(static_cast<size_t>(segmentCount));
    for (int i = 1; i <= segmentCount; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(segmentCount);
        ScreenPoint segPt{};
        segPt.x = static_cast<int32_t>(std::lround(static_cast<double>(from.x) + (totalDx * t)));
        segPt.y = static_cast<int32_t>(std::lround(static_cast<double>(from.y) + (totalDy * t)));
        AppendUniquePoint(&plan.segmentPoints, segPt);
    }
    return plan;
}

} // namespace mousefx::macos_trail_pulse
