#include "pch.h"

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.Shared.h"

#include "MouseFx/Utils/MathUtils.h"

#include <cmath>

namespace mousefx::macos_effect_profile {

ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config) {
    ScrollRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.baseDurationSec = detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 0.8, 0.2, 1.0);
    profile.perStrengthStepSec = detail::ClampDouble(profile.baseDurationSec * 0.065, 0.010, 0.060);
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
    profile.breatheDurationSec =
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.2, 0.45, 2.5);
    profile.rotateDurationSec = detail::ClampDouble(profile.breatheDurationSec * 2.2, 1.0, 3.5);
    profile.rotateDurationFastSec = detail::ClampDouble(profile.rotateDurationSec * 0.68, 0.7, 2.4);
    profile.baseOpacity = 0.92;
    return profile;
}

HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config) {
    HoverRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.sizePx = ClampInt(config.ripple.windowSize + 52, 120, 240);
    profile.breatheDurationSec =
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.1, 0.45, 2.4);
    profile.spinDurationSec = detail::ClampDouble(profile.breatheDurationSec * 2.0, 0.9, 3.6);
    profile.baseOpacity = 0.9;
    return profile;
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
