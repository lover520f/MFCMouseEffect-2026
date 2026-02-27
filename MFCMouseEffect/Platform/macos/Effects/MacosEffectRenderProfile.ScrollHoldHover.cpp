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

} // namespace

ScrollRenderProfile ResolveScrollRenderProfile(const EffectConfig& config) {
    ScrollRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.baseDurationSec = detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 0.92, 0.24, 1.12);
    profile.perStrengthStepSec = detail::ClampDouble(profile.baseDurationSec * 0.072, 0.012, 0.065);
    profile.horizontalSizePx = ClampInt(config.ripple.windowSize + 34, 116, 236);
    profile.verticalSizePx = ClampInt(config.ripple.windowSize + 24, 108, 224);
    profile.closePaddingMs = 90;
    profile.baseOpacity = 0.96;

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
    HoldRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.sizePx = ClampInt(config.ripple.windowSize + 68, 140, 260);
    profile.progressFullMs = ClampInt(static_cast<int>(std::lround(rippleDurationMs * 4.0)), 800, 3000);
    profile.breatheDurationSec =
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.57, 0.55, 2.8);
    profile.rotateDurationSec = detail::ClampDouble(profile.breatheDurationSec * 2.44, 1.2, 4.0);
    profile.rotateDurationFastSec = detail::ClampDouble(profile.rotateDurationSec * 0.68, 0.7, 2.4);
    profile.baseOpacity = 0.92;
    return profile;
}

HoverRenderProfile ResolveHoverRenderProfile(const EffectConfig& config) {
    HoverRenderProfile profile{};
    const int rippleDurationMs = ClampInt(config.ripple.durationMs, 180, 1200);
    profile.sizePx = ClampInt(config.ripple.windowSize + 52, 120, 240);
    profile.breatheDurationSec =
        detail::ClampDouble(static_cast<double>(rippleDurationMs) / 1000.0 * 2.43, 0.55, 2.6);
    profile.spinDurationSec = detail::ClampDouble(profile.breatheDurationSec * 1.88, 1.0, 3.8);
    profile.baseOpacity = 0.9;

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
