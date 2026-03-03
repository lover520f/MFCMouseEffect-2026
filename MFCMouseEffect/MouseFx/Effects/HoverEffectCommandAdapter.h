#pragma once

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <algorithm>
#include <cmath>

namespace mousefx::hover_effect_adapter {
namespace {

inline double ClampOpacityScale(double value) {
    return std::clamp(value, 0.0, 1.0);
}

inline uint32_t ApplyOpacityScale(uint32_t argb, double opacityScale) {
    const uint32_t rgb = argb & 0x00FFFFFFu;
    const int alpha = static_cast<int>((argb >> 24) & 0xFFu);
    const int scaled = static_cast<int>(std::lround(static_cast<double>(alpha) * ClampOpacityScale(opacityScale)));
    return (static_cast<uint32_t>(std::clamp(scaled, 0, 255)) << 24) | rgb;
}

inline uint32_t ResolveGlowColor(uint32_t strokeArgb, uint32_t templateGlowArgb) {
    const uint32_t rgb = strokeArgb & 0x00FFFFFFu;
    const uint32_t alpha = templateGlowArgb & 0xFF000000u;
    return alpha | rgb;
}

} // namespace

inline HoverEffectProfile BuildHoverProfileFromStyle(const RippleStyle& style) {
    HoverEffectProfile profile{};
    profile.sizePx = std::clamp(style.windowSize, 64, 640);
    const double durationSec = std::clamp(static_cast<double>(style.durationMs) / 1000.0, 0.05, 8.0);
    profile.breatheDurationSec = durationSec;
    profile.spinDurationSec = durationSec;
    profile.baseOpacity = 1.0;
    profile.glowSizeScale = 1.0;
    profile.tubesSizeScale = 1.0;
    profile.glowBreatheScale = 1.0;
    profile.tubesBreatheScale = 1.0;
    profile.tubesSpinScale = 1.0;
    profile.colors.glowFillArgb = style.fill.value;
    profile.colors.glowStrokeArgb = style.stroke.value;
    profile.colors.tubesStrokeArgb = style.stroke.value;
    return profile;
}

inline RippleStyle BuildRippleStyleFromCommand(
    const RippleStyle& styleTemplate,
    const HoverEffectRenderCommand& command) {
    RippleStyle style = styleTemplate;
    const double durationSec = std::max(command.breatheDurationSec, command.tubesSpinDurationSec);
    style.durationMs = static_cast<uint32_t>(std::clamp(
        static_cast<int>(std::lround(durationSec * 1000.0)),
        1,
        60000));

    const int templateSize = std::max(styleTemplate.windowSize, 1);
    style.windowSize = std::clamp(command.sizePx, 64, 640);
    const double sizeScale = static_cast<double>(style.windowSize) / static_cast<double>(templateSize);
    style.startRadius = static_cast<float>(std::clamp(
        static_cast<double>(styleTemplate.startRadius) * sizeScale,
        0.0,
        640.0));
    style.endRadius = static_cast<float>(std::clamp(
        static_cast<double>(styleTemplate.endRadius) * sizeScale,
        1.0,
        800.0));
    style.strokeWidth = static_cast<float>(std::clamp(
        static_cast<double>(styleTemplate.strokeWidth) * sizeScale,
        0.1,
        64.0));

    const uint32_t strokeArgb = command.tubesMode ? command.tubesStrokeArgb : command.glowStrokeArgb;
    style.fill.value = ApplyOpacityScale(command.glowFillArgb, command.baseOpacity);
    style.stroke.value = ApplyOpacityScale(strokeArgb, command.baseOpacity);
    style.glow.value = ApplyOpacityScale(ResolveGlowColor(strokeArgb, styleTemplate.glow.value), command.baseOpacity);
    return style;
}

inline ClickEvent BuildClickEventFromCommand(const HoverEffectRenderCommand& command) {
    ClickEvent event{};
    event.pt = command.overlayPoint;
    event.button = MouseButton::Left;
    return event;
}

inline RenderParams BuildRenderParamsFromCommand() {
    RenderParams params{};
    params.loop = true;
    params.intensity = 1.0f;
    return params;
}

} // namespace mousefx::hover_effect_adapter
