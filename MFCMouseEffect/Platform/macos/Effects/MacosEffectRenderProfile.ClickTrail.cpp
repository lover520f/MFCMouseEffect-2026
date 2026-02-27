#include "pch.h"

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.Shared.h"

#include "MouseFx/Utils/MathUtils.h"

#include <cmath>

namespace mousefx::macos_effect_profile {
namespace {

uint8_t ClampToByte(int value) {
    return static_cast<uint8_t>(ClampInt(value, 0, 255));
}

uint32_t ScaleArgbBrightness(uint32_t argb, double scale) {
    const uint8_t alpha = static_cast<uint8_t>((argb >> 24) & 0xFFu);
    const uint8_t red = ClampToByte(static_cast<int>(std::lround(((argb >> 16) & 0xFFu) * scale)));
    const uint8_t green = ClampToByte(static_cast<int>(std::lround(((argb >> 8) & 0xFFu) * scale)));
    const uint8_t blue = ClampToByte(static_cast<int>(std::lround((argb & 0xFFu) * scale)));
    return (static_cast<uint32_t>(alpha) << 24) |
           (static_cast<uint32_t>(red) << 16) |
           (static_cast<uint32_t>(green) << 8) |
           static_cast<uint32_t>(blue);
}

uint32_t WithAlpha(uint32_t argb, uint8_t alpha) {
    return (static_cast<uint32_t>(alpha) << 24) | (argb & 0x00FFFFFFu);
}

uint32_t BlendArgb(uint32_t lhsArgb, uint32_t rhsArgb, double rhsWeight) {
    const double safeWeight = detail::ClampDouble(rhsWeight, 0.0, 1.0);
    const double lhsWeight = 1.0 - safeWeight;
    const auto blendOne = [&](int lhsShift, int rhsShift) {
        const int lhs = static_cast<int>((lhsArgb >> lhsShift) & 0xFFu);
        const int rhs = static_cast<int>((rhsArgb >> rhsShift) & 0xFFu);
        return ClampToByte(static_cast<int>(std::lround(lhs * lhsWeight + rhs * safeWeight)));
    };

    const uint8_t alpha = blendOne(24, 24);
    const uint8_t red = blendOne(16, 16);
    const uint8_t green = blendOne(8, 8);
    const uint8_t blue = blendOne(0, 0);
    return (static_cast<uint32_t>(alpha) << 24) |
           (static_cast<uint32_t>(red) << 16) |
           (static_cast<uint32_t>(green) << 8) |
           static_cast<uint32_t>(blue);
}

double ScaleOpacity(double baseOpacity, double opacityScale) {
    return detail::ScaleDouble(baseOpacity, opacityScale, 0.05, 1.0);
}

} // namespace

ClickRenderProfile ResolveClickRenderProfile(const EffectConfig& config) {
    const TestProfileTuning tuning = ResolveTestProfileTuning();
    ClickRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    const int textDurationMs = ClampInt(config.textClick.durationMs, 220, 1800);
    const int baseWindowSize = ClampInt(config.ripple.windowSize, 80, 220);
    profile.normalSizePx = detail::ScaleInt(baseWindowSize + 22, tuning.sizeScale, 80, 320);
    profile.textSizePx = detail::ScaleInt(baseWindowSize + 38, tuning.sizeScale, 92, 360);
    profile.normalDurationSec = detail::ScaleDouble(
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 1.06, 0.20, 1.30),
        tuning.durationScale,
        0.12,
        2.2);
    profile.textDurationSec = detail::ScaleDouble(
        detail::ClampDouble(static_cast<double>(textDurationMs) / 1000.0 * 0.50, 0.28, 1.35),
        tuning.durationScale,
        0.16,
        2.4);
    profile.closePaddingMs = 60;
    profile.baseOpacity = ScaleOpacity(0.95, tuning.opacityScale);
    profile.leftButton.fillArgb = config.ripple.leftClick.fill.value;
    profile.leftButton.strokeArgb = config.ripple.leftClick.stroke.value;
    profile.leftButton.glowArgb = config.ripple.leftClick.glow.value;
    profile.rightButton.fillArgb = config.ripple.rightClick.fill.value;
    profile.rightButton.strokeArgb = config.ripple.rightClick.stroke.value;
    profile.rightButton.glowArgb = config.ripple.rightClick.glow.value;
    profile.middleButton.fillArgb = config.ripple.middleClick.fill.value;
    profile.middleButton.strokeArgb = config.ripple.middleClick.stroke.value;
    profile.middleButton.glowArgb = config.ripple.middleClick.glow.value;
    return profile;
}

TrailRenderProfile ResolveTrailRenderProfile(const EffectConfig& config, const std::string& trailType) {
    const TestProfileTuning tuning = ResolveTestProfileTuning();
    TrailRenderProfile profile{};
    const std::string normalizedTrailType = detail::NormalizeTrailTypeAlias(trailType);
    const TrailHistoryProfile history = config.GetTrailHistoryProfile(normalizedTrailType);
    const int durationMs = ClampInt(static_cast<int>(std::lround(history.durationMs * 0.73)), 180, 1000);
    profile.durationSec =
        detail::ScaleDouble(static_cast<double>(durationMs) / 1000.0, tuning.durationScale, 0.12, 2.0);
    profile.normalSizePx =
        detail::ScaleInt(ClampInt(56 + history.maxPoints / 3, 56, 112), tuning.sizeScale, 40, 224);
    profile.particleSizePx =
        detail::ScaleInt(ClampInt(40 + history.maxPoints / 6, 40, 72), tuning.sizeScale, 28, 160);
    profile.closePaddingMs = 40;
    profile.baseOpacity = ScaleOpacity(0.95, tuning.opacityScale);
    const uint32_t trailBaseStroke = config.trail.color.value;
    const uint32_t trailBaseFill = WithAlpha(trailBaseStroke, 0x3Du);
    profile.line.strokeArgb = trailBaseStroke;
    profile.line.fillArgb = trailBaseFill;

    profile.streamer.strokeArgb = config.ripple.leftClick.stroke.value;
    profile.streamer.fillArgb = config.ripple.leftClick.fill.value;

    profile.electric.strokeArgb = BlendArgb(config.ripple.leftClick.stroke.value, trailBaseStroke, 0.5);
    profile.electric.fillArgb = WithAlpha(profile.electric.strokeArgb, 0x3D);

    profile.meteor.strokeArgb = config.ripple.rightClick.stroke.value;
    profile.meteor.fillArgb = config.ripple.rightClick.fill.value;

    profile.tubes.strokeArgb = config.ripple.middleClick.stroke.value;
    profile.tubes.fillArgb = config.ripple.middleClick.fill.value;

    profile.particle.strokeArgb = ScaleArgbBrightness(config.ripple.rightClick.stroke.value, 1.08);
    profile.particle.fillArgb = WithAlpha(profile.particle.strokeArgb, 0x3D);
    return profile;
}

TrailThrottleProfile ResolveTrailThrottleProfile(const EffectConfig& config, const std::string& trailType) {
    const TestProfileTuning tuning = ResolveTestProfileTuning();
    const std::string normalizedTrailType = detail::NormalizeTrailTypeAlias(trailType);
    const TrailThrottleProfile base = detail::ResolveTrailThrottleProfileByType(normalizedTrailType);
    const TrailHistoryProfile history = config.GetTrailHistoryProfile(normalizedTrailType);

    TrailThrottleProfile result{};
    const double durationScale = detail::ClampDouble(static_cast<double>(history.durationMs) / 300.0, 0.5, 2.0);
    const double pointsBias =
        detail::ClampDouble((32.0 - static_cast<double>(history.maxPoints)) / 64.0, -0.5, 0.8);

    result.minIntervalMs = static_cast<uint64_t>(detail::ScaleInt(ClampInt(
        static_cast<int>(std::lround(static_cast<double>(base.minIntervalMs) * std::pow(durationScale, 0.35))),
        8,
        40),
        tuning.trailThrottleScale,
        2,
        80));
    result.minDistancePx = detail::ScaleDouble(
        detail::ClampDouble(base.minDistancePx * (1.0 + pointsBias), 2.0, 12.0),
        tuning.trailThrottleScale,
        1.0,
        30.0);
    return result;
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

} // namespace mousefx::macos_effect_profile
