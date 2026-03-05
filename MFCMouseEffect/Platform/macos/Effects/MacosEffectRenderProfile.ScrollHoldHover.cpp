#include "pch.h"

#include "MouseFx/Effects/HoldEffectCommandAdapter.h"
#include "MouseFx/Styles/ThemeStyle.h"
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

double ResolveCategorySizeScale(int percent) {
    return detail::ClampDouble(static_cast<double>(ClampInt(percent, 50, 200)) / 100.0, 0.5, 2.0);
}

} // namespace

ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config) {
    const TestProfileTuning tuning = ResolveTestProfileTuning();
    const double scrollSizeScale = ResolveCategorySizeScale(config.effectSizeScales.scroll);
    ScrollRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    const double rawBaseDurationSec =
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 0.92, 0.24, 1.12);
    const double rawPerStrengthStepSec = detail::ClampDouble(rawBaseDurationSec * 0.072, 0.012, 0.065);
    profile.baseDurationSec = detail::ScaleDouble(
        rawBaseDurationSec,
        tuning.durationScale,
        0.12,
        2.0);
    profile.perStrengthStepSec = detail::ScaleDouble(
        rawPerStrengthStepSec,
        tuning.durationScale,
        0.004,
        0.18);
    profile.horizontalSizePx =
        detail::ScaleInt(ClampInt(config.ripple.windowSize + 34, 116, 236), tuning.sizeScale, 80, 420);
    profile.verticalSizePx =
        detail::ScaleInt(ClampInt(config.ripple.windowSize + 24, 108, 224), tuning.sizeScale, 72, 400);
    profile.horizontalSizePx = detail::ScaleInt(profile.horizontalSizePx, scrollSizeScale, 72, 640);
    profile.verticalSizePx = detail::ScaleInt(profile.verticalSizePx, scrollSizeScale, 72, 640);
    profile.closePaddingMs = 90;
    profile.baseOpacity = ScaleOpacity(0.96, tuning.opacityScale);

    profile.horizontalPositive.fillArgb = config.ripple.leftClick.fill.value;
    profile.horizontalPositive.strokeArgb = config.ripple.leftClick.stroke.value;

    profile.horizontalNegative.fillArgb = ScaleArgbBrightness(config.ripple.leftClick.fill.value, 1.18);
    profile.horizontalNegative.strokeArgb = ScaleArgbBrightness(config.ripple.leftClick.stroke.value, 1.10);

    profile.verticalPositive.fillArgb = config.ripple.middleClick.fill.value;
    profile.verticalPositive.strokeArgb = config.ripple.middleClick.stroke.value;

    profile.verticalNegative.fillArgb = config.ripple.rightClick.fill.value;
    profile.verticalNegative.strokeArgb = config.ripple.rightClick.stroke.value;
    return profile;
}

HoldRenderProfile ResolveHoldRenderProfile(const EffectConfig& config) {
    const TestProfileTuning tuning = ResolveTestProfileTuning();
    const double holdSizeScale = ResolveCategorySizeScale(config.effectSizeScales.hold);
    const RippleStyle holdStyle = GetThemePalette(config.theme).hold;
    const HoldEffectProfile holdComputeProfile = hold_effect_adapter::BuildHoldProfileFromStyle(holdStyle);
    HoldRenderProfile profile{};
    profile.sizePx = detail::ScaleInt(
        ClampInt(holdComputeProfile.sizePx, 64, 640),
        tuning.sizeScale,
        96,
        520);
    profile.sizePx = detail::ScaleInt(profile.sizePx, holdSizeScale, 96, 780);
    profile.progressFullMs = detail::ScaleInt(
        ClampInt(holdComputeProfile.progressFullMs, 1, 60000),
        tuning.durationScale,
        300,
        8000);
    profile.breatheDurationSec = detail::ScaleDouble(
        holdComputeProfile.breatheDurationSec,
        tuning.durationScale,
        0.18,
        6.0);
    profile.rotateDurationSec = detail::ScaleDouble(holdComputeProfile.rotateDurationSec, tuning.durationScale, 0.4, 9.0);
    profile.rotateDurationFastSec = detail::ScaleDouble(
        holdComputeProfile.rotateDurationFastSec,
        tuning.durationScale,
        0.2,
        6.0);
    profile.baseOpacity = ScaleOpacity(holdComputeProfile.baseOpacity, tuning.opacityScale);

    profile.colors.leftBaseStrokeArgb = holdComputeProfile.colors.leftBaseStrokeArgb;
    profile.colors.rightBaseStrokeArgb = holdComputeProfile.colors.rightBaseStrokeArgb;
    profile.colors.middleBaseStrokeArgb = holdComputeProfile.colors.middleBaseStrokeArgb;
    profile.colors.lightningStrokeArgb = holdComputeProfile.colors.lightningStrokeArgb;
    profile.colors.hexStrokeArgb = holdComputeProfile.colors.hexStrokeArgb;
    profile.colors.hologramStrokeArgb = holdComputeProfile.colors.hologramStrokeArgb;
    profile.colors.quantumHaloStrokeArgb = holdComputeProfile.colors.quantumHaloStrokeArgb;
    profile.colors.fluxFieldStrokeArgb = holdComputeProfile.colors.fluxFieldStrokeArgb;
    profile.colors.techNeonStrokeArgb = holdComputeProfile.colors.techNeonStrokeArgb;
    return profile;
}

HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config) {
    const TestProfileTuning tuning = ResolveTestProfileTuning();
    const double hoverSizeScale = ResolveCategorySizeScale(config.effectSizeScales.hover);
    HoverRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    const double rawBreatheDurationSec =
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.43, 0.55, 2.6);
    const double rawSpinDurationSec = detail::ClampDouble(rawBreatheDurationSec * 1.88, 1.0, 3.8);
    profile.sizePx =
        detail::ScaleInt(ClampInt(config.ripple.windowSize + 52, 120, 240), tuning.sizeScale, 84, 460);
    profile.sizePx = detail::ScaleInt(profile.sizePx, hoverSizeScale, 84, 720);
    profile.sizePx = detail::ScaleInt(profile.sizePx, 0.72, 64, 620);
    profile.breatheDurationSec = detail::ScaleDouble(
        rawBreatheDurationSec,
        tuning.durationScale,
        0.18,
        6.0);
    profile.spinDurationSec = detail::ScaleDouble(
        rawSpinDurationSec,
        tuning.durationScale,
        0.35,
        8.0);
    profile.baseOpacity = ScaleOpacity(0.9, tuning.opacityScale);

    profile.colors.glowFillArgb = ScaleArgbBrightness(config.ripple.leftClick.fill.value, 0.92);
    profile.colors.glowStrokeArgb = config.ripple.leftClick.stroke.value;
    profile.colors.tubesStrokeArgb =
        BlendArgb(config.ripple.leftClick.stroke.value, config.ripple.middleClick.stroke.value, 0.50);
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
