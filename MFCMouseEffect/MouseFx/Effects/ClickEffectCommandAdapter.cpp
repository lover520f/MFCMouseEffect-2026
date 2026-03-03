#include "pch.h"

#include "MouseFx/Effects/ClickEffectCommandAdapter.h"

#include <algorithm>
#include <cmath>

namespace mousefx::click_effect_adapter {
namespace {

double ClampOpacityScale(double value) {
    return std::clamp(value, 0.0, 1.0);
}

uint32_t ApplyOpacityScale(uint32_t argb, double opacityScale) {
    const uint32_t rgb = argb & 0x00FFFFFFu;
    const int alpha = static_cast<int>((argb >> 24) & 0xFFu);
    const int scaled = static_cast<int>(std::lround(static_cast<double>(alpha) * ClampOpacityScale(opacityScale)));
    const uint32_t packedAlpha = static_cast<uint32_t>(std::clamp(scaled, 0, 255)) << 24;
    return packedAlpha | rgb;
}

ClickEffectPalette BuildUniformPalette(const RippleStyle& style) {
    return ClickEffectPalette{
        style.fill.value,
        style.stroke.value,
        style.glow.value,
    };
}

} // namespace

ClickEffectProfile BuildClickProfileFromStyle(const RippleStyle& style) {
    ClickEffectProfile profile{};
    profile.normalSizePx = std::clamp(style.windowSize, 64, 640);
    profile.textSizePx = std::clamp(profile.normalSizePx + 14, 80, 720);
    profile.textFontSizePx = std::clamp(static_cast<double>(profile.normalSizePx) * 0.18, 12.0, 220.0);
    profile.textFloatDistancePx = std::clamp(static_cast<double>(style.endRadius) * 1.4, 0.0, 420.0);
    profile.normalDurationSec = std::clamp(static_cast<double>(style.durationMs) / 1000.0, 0.05, 5.0);
    profile.textDurationSec = std::clamp(profile.normalDurationSec + 0.04, 0.08, 5.0);
    profile.closePaddingMs = std::clamp(static_cast<int>(style.durationMs / 5), 40, 180);
    profile.baseOpacity = 1.0;
    profile.rippleStartRadiusPx = std::clamp(static_cast<double>(style.startRadius), 0.0, 640.0);
    profile.rippleEndRadiusPx = std::clamp(static_cast<double>(style.endRadius), 1.0, 800.0);
    profile.rippleStrokeWidthPx = std::clamp(static_cast<double>(style.strokeWidth), 0.1, 64.0);
    profile.starStartRadiusPx = profile.rippleStartRadiusPx;
    profile.starEndRadiusPx = profile.rippleEndRadiusPx;
    profile.starStrokeWidthPx = profile.rippleStrokeWidthPx;

    const ClickEffectPalette palette = BuildUniformPalette(style);
    profile.left = palette;
    profile.right = palette;
    profile.middle = palette;
    return profile;
}

RippleStyle BuildRippleStyleFromCommand(
    const RippleStyle& styleTemplate,
    const ClickEffectRenderCommand& command) {
    RippleStyle style = styleTemplate;
    style.durationMs = static_cast<uint32_t>(std::clamp(
        static_cast<int>(std::lround(command.animationDurationSec * 1000.0)),
        1,
        60000));
    style.windowSize = std::clamp(command.sizePx, 64, 640);
    style.startRadius = static_cast<float>(std::clamp(command.startRadiusPx, 0.0, 640.0));
    style.endRadius = static_cast<float>(std::clamp(command.endRadiusPx, 1.0, 800.0));
    style.strokeWidth = static_cast<float>(std::clamp(command.strokeWidthPx, 0.1, 64.0));
    style.fill.value = ApplyOpacityScale(command.fillArgb, command.baseOpacity);
    style.stroke.value = ApplyOpacityScale(command.strokeArgb, command.baseOpacity);
    style.glow.value = ApplyOpacityScale(command.glowArgb, command.baseOpacity);
    return style;
}

ClickEvent BuildClickEventFromCommand(
    const ClickEvent& sourceEvent,
    const ClickEffectRenderCommand& command) {
    ClickEvent event = sourceEvent;
    event.pt = command.overlayPoint;
    event.button = command.button;
    return event;
}

} // namespace mousefx::click_effect_adapter
