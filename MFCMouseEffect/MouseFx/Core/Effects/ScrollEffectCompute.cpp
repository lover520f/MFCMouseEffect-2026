#include "pch.h"

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>

namespace mousefx {
namespace {

bool ContainsToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

double Clamp01(double value) {
    return std::clamp(value, 0.0, 1.0);
}

double ResolveDurationScale(const ScrollEffectRenderCommand& command, const ScrollEffectProfile& profile) {
    if (command.helixMode) return profile.helixDurationScale;
    if (command.twinkleMode) return profile.twinkleDurationScale;
    return profile.defaultDurationScale;
}

double ResolveSizeScale(const ScrollEffectRenderCommand& command, const ScrollEffectProfile& profile) {
    if (command.helixMode) return profile.helixSizeScale;
    if (command.twinkleMode) return profile.twinkleSizeScale;
    return profile.defaultSizeScale;
}

const ScrollEffectDirectionColorProfile& ResolveDirectionColor(
    bool horizontal,
    int delta,
    const ScrollEffectProfile& profile) {
    if (horizontal) {
        return (delta >= 0) ? profile.horizontalPositive : profile.horizontalNegative;
    }
    return (delta >= 0) ? profile.verticalPositive : profile.verticalNegative;
}

} // namespace

std::string NormalizeScrollEffectType(const std::string& effectType) {
    const std::string lowered = ToLowerAscii(TrimAscii(effectType));
    if (lowered == "none") {
        return "none";
    }
    if (lowered.empty()) {
        return "arrow";
    }
    if (ContainsToken(lowered, "helix")) return "helix";
    if (ContainsToken(lowered, "twinkle") || ContainsToken(lowered, "stardust")) return "twinkle";
    return "arrow";
}

int ResolveScrollStrengthLevel(int delta) {
    const int magnitude = std::abs(delta);
    if (magnitude <= 0) {
        return 0;
    }
    int level = magnitude / 120;
    if (level < 1) {
        level = 1;
    }
    if (level > 6) {
        level = 6;
    }
    return level;
}

ScrollEffectInputShaperProfile ResolveScrollInputShaperProfile(const std::string& effectType) {
    ScrollEffectInputShaperProfile profile{};
    const std::string normalizedType = NormalizeScrollEffectType(effectType);
    if (normalizedType == "helix") {
        profile.emitIntervalMs = 14;
        profile.maxActiveRipples = 8;
        profile.maxDurationMs = 240;
        return profile;
    }
    if (normalizedType == "twinkle") {
        profile.emitIntervalMs = 30;
        profile.maxActiveRipples = 3;
        profile.maxDurationMs = 220;
        return profile;
    }
    return profile;
}

ScrollEffectProfile BuildScrollEffectProfileFromStyle(const RippleStyle& style, int sizeScalePercent) {
    ScrollEffectProfile profile{};
    const double sizeScale = std::clamp(static_cast<double>(sizeScalePercent) / 100.0, 0.5, 2.0);
    const int windowSize = std::clamp(
        static_cast<int>(std::lround(static_cast<double>(std::clamp(style.windowSize, 64, 640)) * sizeScale)),
        64,
        640);
    profile.verticalSizePx = windowSize;
    profile.horizontalSizePx = windowSize;
    profile.geometryReferenceSizePx = windowSize;
    profile.baseStartRadiusPx = std::clamp(static_cast<double>(style.startRadius), 0.0, 640.0);
    profile.baseEndRadiusPx = std::clamp(static_cast<double>(style.endRadius), 1.0, 800.0);
    profile.baseStrokeWidthPx = std::clamp(static_cast<double>(style.strokeWidth), 0.1, 64.0);
    profile.baseDurationSec = std::clamp(static_cast<double>(style.durationMs) / 1000.0, 0.05, 5.0);
    profile.perStrengthStepSec = 0.0;
    profile.closePaddingMs = 0;
    profile.baseOpacity = 1.0;
    profile.defaultDurationScale = 1.0;
    profile.helixDurationScale = 1.0;
    profile.twinkleDurationScale = 1.0;
    profile.defaultSizeScale = 1.0;
    profile.helixSizeScale = 1.0;
    profile.twinkleSizeScale = 1.0;

    const ScrollEffectDirectionColorProfile color{style.fill.value, style.stroke.value};
    profile.horizontalPositive = color;
    profile.horizontalNegative = color;
    profile.verticalPositive = color;
    profile.verticalNegative = color;
    return profile;
}

ScrollEffectDirectionVector ResolveScrollDirectionVector(bool horizontal, int delta) {
    ScrollEffectDirectionVector direction{};
    if (delta == 0) {
        return direction;
    }

    if (horizontal) {
        direction.dx = (delta >= 0) ? 1.0 : -1.0;
        return direction;
    }

    // Screen-space Y grows downward, so "scroll up" maps to negative Y.
    direction.dy = (delta >= 0) ? -1.0 : 1.0;
    return direction;
}

float ResolveScrollDirectionRadians(bool horizontal, int delta) {
    const ScrollEffectDirectionVector direction = ResolveScrollDirectionVector(horizontal, delta);
    if (direction.dx == 0.0 && direction.dy == 0.0) {
        return 0.0f;
    }
    return static_cast<float>(std::atan2(direction.dy, direction.dx));
}

ScrollEffectRenderCommand ComputeScrollEffectRenderCommand(
    const ScreenPoint& overlayPoint,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const ScrollEffectProfile& profile) {
    ScrollEffectRenderCommand command{};
    command.overlayPoint = overlayPoint;
    command.horizontal = horizontal;
    command.delta = delta;
    if (delta == 0) {
        return command;
    }
    command.emit = true;
    command.strengthScalar = static_cast<double>(std::abs(delta)) / 120.0;
    command.intensity = Clamp01(0.6 + command.strengthScalar * 0.6);
    command.strengthLevel = ResolveScrollStrengthLevel(delta);
    command.normalizedType = NormalizeScrollEffectType(effectType);
    command.helixMode = (command.normalizedType == "helix");
    command.twinkleMode = (command.normalizedType == "twinkle");

    const double baseSize = static_cast<double>(horizontal ? profile.horizontalSizePx : profile.verticalSizePx);
    command.sizePx = static_cast<int>(std::lround(
        std::clamp(baseSize * ResolveSizeScale(command, profile), 88.0, 260.0)));
    const double geometryReference = static_cast<double>(std::max(profile.geometryReferenceSizePx, 1));
    const double geometryScale = static_cast<double>(command.sizePx) / geometryReference;
    command.startRadiusPx = std::clamp(profile.baseStartRadiusPx * geometryScale, 0.0, 260.0);
    command.endRadiusPx = std::clamp(profile.baseEndRadiusPx * geometryScale, 1.0, 360.0);
    command.strokeWidthPx = std::clamp(profile.baseStrokeWidthPx * geometryScale, 0.5, 24.0);

    const double baseDuration =
        profile.baseDurationSec + profile.perStrengthStepSec * static_cast<double>(command.strengthLevel);
    command.durationSec = std::clamp(baseDuration * ResolveDurationScale(command, profile), 0.08, 3.0);
    const ScrollEffectInputShaperProfile shaper = ResolveScrollInputShaperProfile(command.normalizedType);
    const double maxDurationSec = static_cast<double>(shaper.maxDurationMs) / 1000.0;
    command.durationSec = std::min(command.durationSec, maxDurationSec);
    command.closeAfterMs = static_cast<int>(command.durationSec * 1000.0) + profile.closePaddingMs;
    command.baseOpacity = std::clamp(profile.baseOpacity, 0.05, 1.0);

    const auto& colors = ResolveDirectionColor(horizontal, delta, profile);
    command.fillArgb = colors.fillArgb;
    command.strokeArgb = colors.strokeArgb;
    return command;
}

} // namespace mousefx
