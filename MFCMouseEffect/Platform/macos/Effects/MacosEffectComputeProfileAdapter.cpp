#include "pch.h"

#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"

namespace mousefx::macos_effect_compute_profile {

ClickEffectProfile BuildClickProfile(const macos_effect_profile::ClickRenderProfile& profile) {
    ClickEffectProfile out{};
    out.normalSizePx = profile.normalSizePx;
    out.textSizePx = profile.textSizePx;
    out.normalDurationSec = profile.normalDurationSec;
    out.textDurationSec = profile.textDurationSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.left = {profile.leftButton.fillArgb, profile.leftButton.strokeArgb, profile.leftButton.glowArgb};
    out.right = {profile.rightButton.fillArgb, profile.rightButton.strokeArgb, profile.rightButton.glowArgb};
    out.middle = {profile.middleButton.fillArgb, profile.middleButton.strokeArgb, profile.middleButton.glowArgb};
    return out;
}

TrailEffectProfile BuildTrailProfile(const macos_effect_profile::TrailRenderProfile& profile) {
    TrailEffectProfile out{};
    out.normalSizePx = profile.normalSizePx;
    out.particleSizePx = profile.particleSizePx;
    out.lineWidthPx = profile.lineWidthPx;
    out.durationSec = profile.durationSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.line = {profile.line.fillArgb, profile.line.strokeArgb};
    out.streamer = {profile.streamer.fillArgb, profile.streamer.strokeArgb};
    out.electric = {profile.electric.fillArgb, profile.electric.strokeArgb};
    out.meteor = {profile.meteor.fillArgb, profile.meteor.strokeArgb};
    out.tubes = {profile.tubes.fillArgb, profile.tubes.strokeArgb};
    out.particle = {profile.particle.fillArgb, profile.particle.strokeArgb};
    out.lineTempo = {profile.lineTempo.durationScale, profile.lineTempo.sizeScale};
    out.streamerTempo = {profile.streamerTempo.durationScale, profile.streamerTempo.sizeScale};
    out.electricTempo = {profile.electricTempo.durationScale, profile.electricTempo.sizeScale};
    out.meteorTempo = {profile.meteorTempo.durationScale, profile.meteorTempo.sizeScale};
    out.tubesTempo = {profile.tubesTempo.durationScale, profile.tubesTempo.sizeScale};
    out.particleTempo = {profile.particleTempo.durationScale, profile.particleTempo.sizeScale};
    return out;
}

TrailEffectThrottleProfile BuildTrailThrottleProfile(const macos_effect_profile::TrailThrottleProfile& profile) {
    TrailEffectThrottleProfile out{};
    out.minIntervalMs = profile.minIntervalMs;
    out.minDistancePx = profile.minDistancePx;
    return out;
}

ScrollEffectProfile BuildScrollProfile(const macos_effect_profile::ScrollRenderProfile& profile) {
    ScrollEffectProfile out{};
    out.verticalSizePx = profile.verticalSizePx;
    out.horizontalSizePx = profile.horizontalSizePx;
    out.baseDurationSec = profile.baseDurationSec;
    out.perStrengthStepSec = profile.perStrengthStepSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.defaultDurationScale = profile.defaultDurationScale;
    out.helixDurationScale = profile.helixDurationScale;
    out.twinkleDurationScale = profile.twinkleDurationScale;
    out.defaultSizeScale = profile.defaultSizeScale;
    out.helixSizeScale = profile.helixSizeScale;
    out.twinkleSizeScale = profile.twinkleSizeScale;
    out.horizontalPositive = {profile.horizontalPositive.fillArgb, profile.horizontalPositive.strokeArgb};
    out.horizontalNegative = {profile.horizontalNegative.fillArgb, profile.horizontalNegative.strokeArgb};
    out.verticalPositive = {profile.verticalPositive.fillArgb, profile.verticalPositive.strokeArgb};
    out.verticalNegative = {profile.verticalNegative.fillArgb, profile.verticalNegative.strokeArgb};
    return out;
}

HoverEffectProfile BuildHoverProfile(const macos_effect_profile::HoverRenderProfile& profile) {
    HoverEffectProfile out{};
    out.sizePx = profile.sizePx;
    out.breatheDurationSec = profile.breatheDurationSec;
    out.spinDurationSec = profile.spinDurationSec;
    out.baseOpacity = profile.baseOpacity;
    out.glowSizeScale = profile.glowSizeScale;
    out.tubesSizeScale = profile.tubesSizeScale;
    out.glowBreatheScale = profile.glowBreatheScale;
    out.tubesBreatheScale = profile.tubesBreatheScale;
    out.tubesSpinScale = profile.tubesSpinScale;
    out.colors = {profile.colors.glowFillArgb, profile.colors.glowStrokeArgb, profile.colors.tubesStrokeArgb};
    return out;
}

HoldEffectProfile BuildHoldProfile(const macos_effect_profile::HoldRenderProfile& profile) {
    HoldEffectProfile out{};
    out.sizePx = profile.sizePx;
    out.progressFullMs = profile.progressFullMs;
    out.breatheDurationSec = profile.breatheDurationSec;
    out.rotateDurationSec = profile.rotateDurationSec;
    out.rotateDurationFastSec = profile.rotateDurationFastSec;
    out.baseOpacity = profile.baseOpacity;
    out.colors.leftBaseStrokeArgb = profile.colors.leftBaseStrokeArgb;
    out.colors.rightBaseStrokeArgb = profile.colors.rightBaseStrokeArgb;
    out.colors.middleBaseStrokeArgb = profile.colors.middleBaseStrokeArgb;
    out.colors.lightningStrokeArgb = profile.colors.lightningStrokeArgb;
    out.colors.hexStrokeArgb = profile.colors.hexStrokeArgb;
    out.colors.hologramStrokeArgb = profile.colors.hologramStrokeArgb;
    out.colors.quantumHaloStrokeArgb = profile.colors.quantumHaloStrokeArgb;
    out.colors.fluxFieldStrokeArgb = profile.colors.fluxFieldStrokeArgb;
    out.colors.techNeonStrokeArgb = profile.colors.techNeonStrokeArgb;
    return out;
}

} // namespace mousefx::macos_effect_compute_profile
