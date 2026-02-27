#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

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

inline bool ContainsTrailToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

inline std::string NormalizeTrailTypeAlias(std::string type) {
    type = ToLowerAscii(type);
    if (type == "scifi" || type == "sci-fi" || type == "sci_fi") {
        return "tubes";
    }
    if (ContainsTrailToken(type, "meteor")) {
        return "meteor";
    }
    if (ContainsTrailToken(type, "streamer") || ContainsTrailToken(type, "stream") || ContainsTrailToken(type, "neon")) {
        return "streamer";
    }
    if (ContainsTrailToken(type, "electric") || ContainsTrailToken(type, "arc")) {
        return "electric";
    }
    if (ContainsTrailToken(type, "tube") || ContainsTrailToken(type, "suspension")) {
        return "tubes";
    }
    if (ContainsTrailToken(type, "line") || ContainsTrailToken(type, "default")) {
        return "line";
    }
    if (type == "particle" || ContainsTrailToken(type, "spark")) {
        return "particle";
    }
    return type;
}

inline TrailThrottleProfile ResolveTrailThrottleProfileByType(const std::string& trailType) {
    const std::string type = NormalizeTrailTypeAlias(trailType);
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

inline bool TryParseEnvDouble(const char* key, double* outValue) {
    if (outValue == nullptr) {
        return false;
    }
    const char* raw = std::getenv(key);
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }
    char* end = nullptr;
    const double value = std::strtod(raw, &end);
    if (end == raw || (end != nullptr && *end != '\0')) {
        return false;
    }
    *outValue = value;
    return true;
}

inline TestProfileTuning ResolveTestProfileTuningFromEnv() {
    constexpr const char* kDurationScaleEnv = "MFX_TEST_EFFECTS_DURATION_SCALE";
    constexpr const char* kSizeScaleEnv = "MFX_TEST_EFFECTS_SIZE_SCALE";
    constexpr const char* kOpacityScaleEnv = "MFX_TEST_EFFECTS_OPACITY_SCALE";
    constexpr const char* kTrailThrottleScaleEnv = "MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE";

    TestProfileTuning tuning{};
    double parsed = 0.0;
    if (TryParseEnvDouble(kDurationScaleEnv, &parsed)) {
        tuning.durationScale = ClampDouble(parsed, 0.25, 3.0);
        tuning.durationOverridden = true;
    }
    if (TryParseEnvDouble(kSizeScaleEnv, &parsed)) {
        tuning.sizeScale = ClampDouble(parsed, 0.5, 2.0);
        tuning.sizeOverridden = true;
    }
    if (TryParseEnvDouble(kOpacityScaleEnv, &parsed)) {
        tuning.opacityScale = ClampDouble(parsed, 0.25, 1.5);
        tuning.opacityOverridden = true;
    }
    if (TryParseEnvDouble(kTrailThrottleScaleEnv, &parsed)) {
        tuning.trailThrottleScale = ClampDouble(parsed, 0.25, 4.0);
        tuning.trailThrottleOverridden = true;
    }
    return tuning;
}

inline int ScaleInt(int value, double scale, int lo, int hi) {
    return std::clamp(static_cast<int>(std::lround(static_cast<double>(value) * scale)), lo, hi);
}

inline double ScaleDouble(double value, double scale, double lo, double hi) {
    return ClampDouble(value * scale, lo, hi);
}

} // namespace mousefx::macos_effect_profile::detail
