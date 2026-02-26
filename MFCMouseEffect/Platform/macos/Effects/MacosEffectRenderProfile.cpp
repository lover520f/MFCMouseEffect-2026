#include "pch.h"

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <cmath>

namespace mousefx::macos_effect_profile {
namespace {

double ClampDouble(double value, double lo, double hi) {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

TrailThrottleProfile ResolveTrailThrottleProfileByType(const std::string& trailType) {
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

} // namespace

ClickRenderProfile ResolveClickRenderProfile(const EffectConfig& config) {
    ClickRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    const int textDurationMs = ClampInt(config.textClick.durationMs, 220, 1800);
    const int baseWindowSize = ClampInt(config.ripple.windowSize, 80, 220);
    profile.normalSizePx = baseWindowSize + 18;
    profile.textSizePx = baseWindowSize + 32;
    profile.normalDurationSec = static_cast<double>(rippleDurationMs) / 1000.0;
    profile.textDurationSec = ClampDouble(static_cast<double>(textDurationMs) / 1000.0 * 0.45, 0.24, 1.2);
    profile.closePaddingMs = 60;
    profile.baseOpacity = 0.95;
    return profile;
}

TrailRenderProfile ResolveTrailRenderProfile(const EffectConfig& config, const std::string& trailType) {
    TrailRenderProfile profile{};
    const TrailHistoryProfile history = config.GetTrailHistoryProfile(trailType);
    const int durationMs = ClampInt(static_cast<int>(std::lround(history.durationMs * 0.55)), 140, 900);
    profile.durationSec = static_cast<double>(durationMs) / 1000.0;
    profile.normalSizePx = ClampInt(56 + history.maxPoints / 3, 56, 112);
    profile.particleSizePx = ClampInt(40 + history.maxPoints / 6, 40, 72);
    profile.closePaddingMs = 40;
    profile.baseOpacity = 0.95;
    return profile;
}

TrailThrottleProfile ResolveTrailThrottleProfile(const EffectConfig& config, const std::string& trailType) {
    const TrailThrottleProfile base = ResolveTrailThrottleProfileByType(trailType);
    const TrailHistoryProfile history = config.GetTrailHistoryProfile(trailType);

    TrailThrottleProfile result{};
    const double durationScale = ClampDouble(static_cast<double>(history.durationMs) / 300.0, 0.5, 2.0);
    const double pointsBias = ClampDouble((32.0 - static_cast<double>(history.maxPoints)) / 64.0, -0.5, 0.8);

    result.minIntervalMs = static_cast<uint64_t>(ClampInt(
        static_cast<int>(std::lround(static_cast<double>(base.minIntervalMs) * std::pow(durationScale, 0.35))),
        8,
        40));
    result.minDistancePx = ClampDouble(base.minDistancePx * (1.0 + pointsBias), 2.0, 12.0);
    return result;
}

ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config) {
    ScrollRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.baseDurationSec = ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 0.8, 0.2, 1.0);
    profile.perStrengthStepSec = ClampDouble(profile.baseDurationSec * 0.065, 0.010, 0.060);
    profile.horizontalSizePx = ClampInt(config.ripple.windowSize + 28, 112, 220);
    profile.verticalSizePx = ClampInt(config.ripple.windowSize + 18, 102, 210);
    profile.closePaddingMs = 90;
    profile.baseOpacity = 0.96;
    return profile;
}

HoldRenderProfile ResolveHoldRenderProfile(const EffectConfig& config) {
    HoldRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.sizePx = ClampInt(config.ripple.windowSize + 68, 140, 260);
    profile.progressFullMs = ClampInt(static_cast<int>(std::lround(rippleDurationMs * 4.0)), 800, 3000);
    profile.breatheDurationSec = ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.2, 0.45, 2.5);
    profile.rotateDurationSec = ClampDouble(profile.breatheDurationSec * 2.2, 1.0, 3.5);
    profile.rotateDurationFastSec = ClampDouble(profile.rotateDurationSec * 0.68, 0.7, 2.4);
    profile.baseOpacity = 0.92;
    return profile;
}

HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config) {
    HoverRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.sizePx = ClampInt(config.ripple.windowSize + 52, 120, 240);
    profile.breatheDurationSec = ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.1, 0.45, 2.4);
    profile.spinDurationSec = ClampDouble(profile.breatheDurationSec * 2.0, 0.9, 3.6);
    profile.baseOpacity = 0.9;
    return profile;
}

ClickRenderProfile DefaultClickRenderProfile() {
    return ResolveClickRenderProfile(EffectConfig::GetDefault());
}

TrailRenderProfile DefaultTrailRenderProfile(const std::string& trailType) {
    return ResolveTrailRenderProfile(EffectConfig::GetDefault(), trailType);
}

TrailThrottleProfile DefaultTrailThrottleProfile(const std::string& trailType) {
    return ResolveTrailThrottleProfile(EffectConfig::GetDefault(), trailType);
}

ScrollRenderProfile DefaultScrollRenderProfile() {
    return ResolveScrollRenderProfile(EffectConfig::GetDefault());
}

HoldRenderProfile DefaultHoldRenderProfile() {
    return ResolveHoldRenderProfile(EffectConfig::GetDefault());
}

HoverRenderProfile DefaultHoverRenderProfile() {
    return ResolveHoverRenderProfile(EffectConfig::GetDefault());
}

} // namespace mousefx::macos_effect_profile
