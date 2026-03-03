#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Styles/RippleStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>

namespace mousefx::hold_effect_adapter {
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

inline bool ContainsToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

inline uint32_t ResolveStrokeColor(const HoldEffectStartCommand& command) {
    const std::string type = ToLowerAscii(command.normalizedType);
    if (ContainsToken(type, "lightning")) {
        return command.colors.lightningStrokeArgb;
    }
    if (ContainsToken(type, "hex")) {
        return command.colors.hexStrokeArgb;
    }
    if (ContainsToken(type, "hologram") || ContainsToken(type, "scifi3d")) {
        return command.colors.hologramStrokeArgb;
    }
    if (ContainsToken(type, "hold_quantum_halo_gpu_v2") ||
        ContainsToken(type, "hold_neon3d_gpu_v2") ||
        ContainsToken(type, "quantum_halo")) {
        return command.colors.quantumHaloStrokeArgb;
    }
    if (ContainsToken(type, "fluxfield") ||
        ContainsToken(type, "flux_field") ||
        ContainsToken(type, "hold_flux_field")) {
        return command.colors.fluxFieldStrokeArgb;
    }
    if (ContainsToken(type, "tech") || ContainsToken(type, "neon")) {
        return command.colors.techNeonStrokeArgb;
    }
    if (command.button == MouseButton::Right) {
        return command.colors.rightBaseStrokeArgb;
    }
    if (command.button == MouseButton::Middle) {
        return command.colors.middleBaseStrokeArgb;
    }
    return command.colors.leftBaseStrokeArgb;
}

inline uint32_t ResolveGlowColor(uint32_t strokeArgb, uint32_t templateGlowArgb) {
    const uint32_t rgb = strokeArgb & 0x00FFFFFFu;
    const uint32_t alpha = templateGlowArgb & 0xFF000000u;
    return alpha | rgb;
}

} // namespace

inline HoldEffectProfile BuildHoldProfileFromStyle(const RippleStyle& style) {
    HoldEffectProfile profile{};
    profile.sizePx = std::clamp(style.windowSize, 64, 640);
    profile.progressFullMs = std::clamp(static_cast<int>(style.durationMs), 1, 60000);
    const double durationSec = std::clamp(static_cast<double>(style.durationMs) / 1000.0, 0.05, 8.0);
    profile.breatheDurationSec = durationSec;
    profile.rotateDurationSec = std::clamp(durationSec * 2.4, 0.2, 10.0);
    profile.rotateDurationFastSec = std::clamp(profile.rotateDurationSec * 0.68, 0.2, 6.0);
    profile.baseOpacity = 1.0;
    profile.colors.leftBaseStrokeArgb = style.stroke.value;
    profile.colors.rightBaseStrokeArgb = style.stroke.value;
    profile.colors.middleBaseStrokeArgb = style.stroke.value;
    profile.colors.lightningStrokeArgb = style.stroke.value;
    profile.colors.hexStrokeArgb = style.stroke.value;
    profile.colors.hologramStrokeArgb = style.stroke.value;
    profile.colors.quantumHaloStrokeArgb = style.stroke.value;
    profile.colors.fluxFieldStrokeArgb = style.stroke.value;
    profile.colors.techNeonStrokeArgb = style.stroke.value;
    return profile;
}

inline RippleStyle BuildRuntimeStyleFromStartCommand(
    const RippleStyle& styleTemplate,
    const HoldEffectStartCommand& command) {
    RippleStyle style = styleTemplate;
    style.durationMs = static_cast<uint32_t>(std::clamp(command.progressFullMs, 1, 60000));
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

    const uint32_t strokeArgb = ResolveStrokeColor(command);
    style.fill.value = ApplyOpacityScale(styleTemplate.fill.value, command.baseOpacity);
    style.stroke.value = ApplyOpacityScale(strokeArgb, command.baseOpacity);
    style.glow.value = ApplyOpacityScale(ResolveGlowColor(strokeArgb, styleTemplate.glow.value), command.baseOpacity);
    return style;
}

} // namespace mousefx::hold_effect_adapter
