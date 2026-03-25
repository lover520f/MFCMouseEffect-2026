#pragma once

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <algorithm>
#include <cmath>

namespace mousefx::scroll_effect_adapter {
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

inline uint32_t ResolveColorWithFallback(uint32_t primaryArgb, uint32_t fallbackArgb) {
    if ((primaryArgb & 0x00FFFFFFu) == 0 && (primaryArgb >> 24) == 0) {
        return fallbackArgb;
    }
    return primaryArgb;
}

inline uint32_t ResolveGlowColor(uint32_t strokeArgb, uint32_t templateGlowArgb) {
    const uint32_t rgb = strokeArgb & 0x00FFFFFFu;
    const uint32_t alpha = templateGlowArgb & 0xFF000000u;
    return alpha | rgb;
}

} // namespace

inline RippleStyle BuildRippleStyleFromCommand(
    const RippleStyle& styleTemplate,
    const ScrollEffectRenderCommand& command) {
    RippleStyle style = styleTemplate;
    style.durationMs = static_cast<uint32_t>(std::clamp(
        static_cast<int>(std::lround(command.durationSec * 1000.0)),
        1,
        60000));

    style.windowSize = std::clamp(command.sizePx, 64, 640);
    const int templateSize = std::max(styleTemplate.windowSize, 1);
    const double sizeScale = static_cast<double>(style.windowSize) / static_cast<double>(templateSize);
    const double fallbackStartRadius = std::clamp(
        static_cast<double>(styleTemplate.startRadius) * sizeScale,
        0.0,
        640.0);
    const double fallbackEndRadius = std::clamp(
        static_cast<double>(styleTemplate.endRadius) * sizeScale,
        1.0,
        800.0);
    const double fallbackStrokeWidth = std::clamp(
        static_cast<double>(styleTemplate.strokeWidth) * sizeScale,
        0.1,
        64.0);
    style.startRadius = static_cast<float>(command.startRadiusPx > 0.0 ? command.startRadiusPx : fallbackStartRadius);
    style.endRadius = static_cast<float>(command.endRadiusPx > 0.0 ? command.endRadiusPx : fallbackEndRadius);
    style.strokeWidth = static_cast<float>(command.strokeWidthPx > 0.0 ? command.strokeWidthPx : fallbackStrokeWidth);

    const uint32_t fillArgb = ResolveColorWithFallback(command.fillArgb, styleTemplate.fill.value);
    const uint32_t strokeArgb = ResolveColorWithFallback(command.strokeArgb, styleTemplate.stroke.value);
    style.fill.value = ApplyOpacityScale(fillArgb, command.baseOpacity);
    style.stroke.value = ApplyOpacityScale(strokeArgb, command.baseOpacity);
    style.glow.value = ApplyOpacityScale(ResolveGlowColor(strokeArgb, styleTemplate.glow.value), command.baseOpacity);
    return style;
}

inline ClickEvent BuildClickEventFromCommand(
    const ScrollEvent& sourceEvent,
    const ScrollEffectRenderCommand& command) {
    ClickEvent event{};
    event.pt = command.overlayPoint;
    event.button = MouseButton::Left;
    if (sourceEvent.delta == 0) {
        event.pt = sourceEvent.pt;
    }
    return event;
}

inline RenderParams BuildRenderParamsFromCommand(const ScrollEffectRenderCommand& command) {
    RenderParams params{};
    params.directionRad = ResolveScrollDirectionRadians(command.horizontal, command.delta);
    params.intensity = static_cast<float>(std::clamp(command.intensity, 0.0, 1.0));
    params.loop = false;
    return params;
}

inline const char* ResolveRendererName(const ScrollEffectRenderCommand& command) {
    if (command.helixMode) {
        return "helix";
    }
    if (command.twinkleMode) {
        return "twinkle";
    }
    return "arrow";
}

} // namespace mousefx::scroll_effect_adapter
