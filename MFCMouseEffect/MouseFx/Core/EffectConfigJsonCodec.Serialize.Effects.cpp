#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildActiveEffectsJson(const ActiveEffectConfig& active) {
    return {
        {"click", active.click},
        {"trail", active.trail},
        {"scroll", active.scroll},
        {"hover", active.hover},
        {"hold", active.hold}
    };
}

nlohmann::json BuildEffectsJson(const EffectConfig& config) {
    nlohmann::json effects;

    effects["ripple"] = {
        {"duration_ms", config.ripple.durationMs},
        {"start_radius", config.ripple.startRadius},
        {"end_radius", config.ripple.endRadius},
        {"stroke_width", config.ripple.strokeWidth},
        {"window_size", config.ripple.windowSize},
        {"left_click", {
            {"fill", config_internal::ArgbToHex(config.ripple.leftClick.fill)},
            {"stroke", config_internal::ArgbToHex(config.ripple.leftClick.stroke)},
            {"glow", config_internal::ArgbToHex(config.ripple.leftClick.glow)}
        }},
        {"right_click", {
            {"fill", config_internal::ArgbToHex(config.ripple.rightClick.fill)},
            {"stroke", config_internal::ArgbToHex(config.ripple.rightClick.stroke)},
            {"glow", config_internal::ArgbToHex(config.ripple.rightClick.glow)}
        }},
        {"middle_click", {
            {"fill", config_internal::ArgbToHex(config.ripple.middleClick.fill)},
            {"stroke", config_internal::ArgbToHex(config.ripple.middleClick.stroke)},
            {"glow", config_internal::ArgbToHex(config.ripple.middleClick.glow)}
        }}
    };

    effects["trail"] = {
        {"duration_ms", config.trail.durationMs},
        {"max_points", config.trail.maxPoints},
        {"line_width", config.trail.lineWidth},
        {"color", config_internal::ArgbToHex(config.trail.color)}
    };

    effects["icon_star"] = {
        {"duration_ms", config.icon.durationMs},
        {"start_radius", config.icon.startRadius},
        {"end_radius", config.icon.endRadius},
        {"stroke_width", config.icon.strokeWidth},
        {"fill", config_internal::ArgbToHex(config.icon.fillColor)},
        {"stroke", config_internal::ArgbToHex(config.icon.strokeColor)}
    };

    nlohmann::json textClick;
    textClick["duration_ms"] = config.textClick.durationMs;
    textClick["float_distance"] = config.textClick.floatDistance;
    textClick["font_size"] = config.textClick.fontSize;
    textClick["font_family"] = config_internal::WStringToUtf8(config.textClick.fontFamily);

    nlohmann::json texts = nlohmann::json::array();
    for (const auto& text : config.textClick.texts) {
        texts.push_back(config_internal::WStringToUtf8(text));
    }
    textClick["texts"] = texts;

    nlohmann::json colors = nlohmann::json::array();
    for (const auto& color : config.textClick.colors) {
        colors.push_back(config_internal::ArgbToHex(color));
    }
    textClick["colors"] = colors;
    effects["text_click"] = textClick;

    return effects;
}

} // namespace mousefx::config_json::serialize_internal
