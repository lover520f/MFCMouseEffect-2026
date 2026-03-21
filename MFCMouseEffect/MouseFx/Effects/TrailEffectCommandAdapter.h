#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Styles/ThemeStyle.h"

#include <algorithm>
#include <cmath>

namespace mousefx::trail_effect_adapter {

inline TrailEffectThrottleProfile ResolveTrailThrottleProfile() {
    TrailEffectThrottleProfile throttle{};
    throttle.minIntervalMs = 18;
    throttle.minDistancePx = 8.0;
    return throttle;
}

inline TrailEffectProfile BuildTrailProfileFromConfig(
    const EffectConfig& config,
    const std::string& normalizedType,
    int historyDurationMs) {
    TrailEffectProfile profile{};
    const RippleStyle style = GetThemePalette(config.theme).click;
    const int trailScalePercent = std::clamp(config.effectSizeScales.trail, 50, 200);
    const double sizeScale = static_cast<double>(trailScalePercent) / 100.0;
    const int baseSize = std::clamp(style.windowSize, 36, 220);
    const int scaledSize = std::clamp(
        static_cast<int>(std::lround(static_cast<double>(baseSize) * sizeScale)),
        28,
        240);
    profile.normalSizePx = scaledSize;
    profile.particleSizePx = std::clamp(
        static_cast<int>(std::lround(static_cast<double>(scaledSize) * 0.78)),
        24,
        200);

    const double width = config.trail.lineWidth > 0.0f
        ? static_cast<double>(config.trail.lineWidth)
        : static_cast<double>(style.strokeWidth);
    profile.lineWidthPx = std::clamp(width, 1.0, 24.0);

    profile.durationSec = std::clamp(
        static_cast<double>(historyDurationMs) / 1000.0,
        0.08,
        3.0);
    profile.closePaddingMs = 40;

    const double alphaScale = static_cast<double>((config.trail.color.value >> 24) & 0xFFu) / 255.0;
    profile.baseOpacity = std::clamp(alphaScale > 0.0 ? alphaScale : 1.0, 0.05, 1.0);

    const uint32_t trailStroke = config.trail.color.value;
    const uint32_t fillFallback = style.fill.value != 0
        ? style.fill.value
        : (0x66u << 24) | (trailStroke & 0x00FFFFFFu);

    profile.line = {fillFallback, trailStroke};
    profile.streamer = {fillFallback, trailStroke};
    profile.electric = {fillFallback, trailStroke};
    profile.meteor = {fillFallback, trailStroke};
    profile.tubes = {fillFallback, trailStroke};
    profile.particle = {fillFallback, trailStroke};

    const auto tempoDefault = TrailEffectTypeTempoProfile{1.0, 1.0};
    profile.lineTempo = tempoDefault;
    profile.streamerTempo = tempoDefault;
    profile.electricTempo = tempoDefault;
    profile.meteorTempo = tempoDefault;
    profile.tubesTempo = tempoDefault;
    profile.particleTempo = tempoDefault;

    if (normalizedType == "tubes") {
        profile.tubesTempo.sizeScale = 1.1;
    } else if (normalizedType == "particle") {
        profile.particleTempo.sizeScale = 0.86;
    }
    return profile;
}

inline TrailPoint BuildTrailPointFromCommand(const TrailEffectRenderCommand& command, uint64_t nowMs) {
    TrailPoint point{};
    point.pt = command.overlayPoint;
    point.addedTime = nowMs;
    point.fillArgb = command.fillArgb;
    point.strokeArgb = command.strokeArgb;
    point.lineWidthPx = command.lineWidthPx;
    point.intensity = command.intensity;
    point.durationMs = static_cast<int>(std::lround(command.durationSec * 1000.0));
    point.durationMs = std::clamp(point.durationMs, 80, 2000);
    return point;
}

} // namespace mousefx::trail_effect_adapter
