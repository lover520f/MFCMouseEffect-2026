#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"

#include <cstdint>
#include <string>

namespace mousefx::macos_effect_profile {

struct ClickRenderProfile {
    int normalSizePx = 138;
    int textSizePx = 152;
    double normalDurationSec = 0.32;
    double textDurationSec = 0.36;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
};

struct TrailRenderProfile {
    int normalSizePx = 64;
    int particleSizePx = 48;
    double durationSec = 0.22;
    int closePaddingMs = 40;
    double baseOpacity = 0.95;
};

struct TrailThrottleProfile {
    uint64_t minIntervalMs = 18;
    double minDistancePx = 8.0;
};

struct ScrollRenderProfile {
    int verticalSizePx = 138;
    int horizontalSizePx = 148;
    double baseDurationSec = 0.28;
    double perStrengthStepSec = 0.018;
    int closePaddingMs = 90;
    double baseOpacity = 0.96;
};

struct HoldRenderProfile {
    int sizePx = 188;
    int progressFullMs = 1400;
    double breatheDurationSec = 0.9;
    double rotateDurationSec = 2.2;
    double rotateDurationFastSec = 1.5;
    double baseOpacity = 0.92;
};

struct HoverRenderProfile {
    int sizePx = 172;
    double breatheDurationSec = 0.85;
    double spinDurationSec = 1.6;
    double baseOpacity = 0.9;
};

ClickRenderProfile ResolveClickRenderProfile(const EffectConfig& config);
TrailRenderProfile ResolveTrailRenderProfile(const EffectConfig& config, const std::string& trailType);
TrailThrottleProfile ResolveTrailThrottleProfile(const EffectConfig& config, const std::string& trailType);
ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config);
HoldRenderProfile ResolveHoldRenderProfile(const EffectConfig& config);
HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config);

ClickRenderProfile DefaultClickRenderProfile();
TrailRenderProfile DefaultTrailRenderProfile(const std::string& trailType);
TrailThrottleProfile DefaultTrailThrottleProfile(const std::string& trailType);
ScrollRenderProfile DefaultScrollRenderProfile();
HoldRenderProfile DefaultHoldRenderProfile();
HoverRenderProfile DefaultHoverRenderProfile();

} // namespace mousefx::macos_effect_profile
