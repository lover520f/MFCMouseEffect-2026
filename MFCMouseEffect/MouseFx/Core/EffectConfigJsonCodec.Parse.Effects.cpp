#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

namespace mousefx::config_json::parse_internal {
namespace {

void ParseRippleEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains("ripple") || !effects["ripple"].is_object()) {
        return;
    }

    const auto& ripple = effects["ripple"];
    config.ripple.durationMs = GetOr<int>(ripple, "duration_ms", config.ripple.durationMs);
    config.ripple.startRadius = GetOr<float>(ripple, "start_radius", config.ripple.startRadius);
    config.ripple.endRadius = GetOr<float>(ripple, "end_radius", config.ripple.endRadius);
    config.ripple.strokeWidth = GetOr<float>(ripple, "stroke_width", config.ripple.strokeWidth);
    config.ripple.windowSize = GetOr<int>(ripple, "window_size", config.ripple.windowSize);

    if (ripple.contains("left_click") && ripple["left_click"].is_object()) {
        const auto& left = ripple["left_click"];
        config.ripple.leftClick.fill = GetColorOr(left, "fill", config.ripple.leftClick.fill);
        config.ripple.leftClick.stroke = GetColorOr(left, "stroke", config.ripple.leftClick.stroke);
        config.ripple.leftClick.glow = GetColorOr(left, "glow", config.ripple.leftClick.glow);
    }
    if (ripple.contains("right_click") && ripple["right_click"].is_object()) {
        const auto& right = ripple["right_click"];
        config.ripple.rightClick.fill = GetColorOr(right, "fill", config.ripple.rightClick.fill);
        config.ripple.rightClick.stroke = GetColorOr(right, "stroke", config.ripple.rightClick.stroke);
        config.ripple.rightClick.glow = GetColorOr(right, "glow", config.ripple.rightClick.glow);
    }
    if (ripple.contains("middle_click") && ripple["middle_click"].is_object()) {
        const auto& middle = ripple["middle_click"];
        config.ripple.middleClick.fill = GetColorOr(middle, "fill", config.ripple.middleClick.fill);
        config.ripple.middleClick.stroke = GetColorOr(middle, "stroke", config.ripple.middleClick.stroke);
        config.ripple.middleClick.glow = GetColorOr(middle, "glow", config.ripple.middleClick.glow);
    }
}

void ParseTrailEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains("trail") || !effects["trail"].is_object()) {
        return;
    }

    const auto& trail = effects["trail"];
    config.trail.durationMs = GetOr<int>(trail, "duration_ms", config.trail.durationMs);
    config.trail.maxPoints = GetOr<int>(trail, "max_points", config.trail.maxPoints);
    config.trail.lineWidth = GetOr<float>(trail, "line_width", config.trail.lineWidth);
    config.trail.color = GetColorOr(trail, "color", config.trail.color);
}

void ParseIconEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains("icon_star") || !effects["icon_star"].is_object()) {
        return;
    }

    const auto& icon = effects["icon_star"];
    config.icon.durationMs = GetOr<int>(icon, "duration_ms", config.icon.durationMs);
    config.icon.startRadius = GetOr<float>(icon, "start_radius", config.icon.startRadius);
    config.icon.endRadius = GetOr<float>(icon, "end_radius", config.icon.endRadius);
    config.icon.strokeWidth = GetOr<float>(icon, "stroke_width", config.icon.strokeWidth);
    config.icon.fillColor = GetColorOr(icon, "fill", config.icon.fillColor);
    config.icon.strokeColor = GetColorOr(icon, "stroke", config.icon.strokeColor);
}

void ParseTextEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains("text_click") || !effects["text_click"].is_object()) {
        return;
    }

    const auto& text = effects["text_click"];
    config.textClick.durationMs = GetOr<int>(text, "duration_ms", config.textClick.durationMs);
    config.textClick.floatDistance = GetOr<int>(text, "float_distance", config.textClick.floatDistance);
    config.textClick.fontSize = GetOr<float>(text, "font_size", config.textClick.fontSize);

    if (text.contains("font_family") && text["font_family"].is_string()) {
        std::wstring fontFamily;
        if (TryUtf8ToWide(text["font_family"].get<std::string>(), &fontFamily)) {
            config.textClick.fontFamily = fontFamily;
        }
    }

    if (text.contains("texts") && text["texts"].is_array()) {
        config.textClick.texts.clear();
        for (const auto& item : text["texts"]) {
            if (!item.is_string()) {
                continue;
            }

            std::wstring ws;
            if (TryUtf8ToWide(item.get<std::string>(), &ws)) {
                config.textClick.texts.push_back(ws);
            }
        }
    }

    if (text.contains("colors") && text["colors"].is_array()) {
        config.textClick.colors.clear();
        for (const auto& item : text["colors"]) {
            if (item.is_string()) {
                config.textClick.colors.push_back(ArgbFromHex(item.get<std::string>()));
            }
        }
    }
}

} // namespace

void ParseEffects(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains("effects") || !root["effects"].is_object()) {
        return;
    }

    const auto& effects = root["effects"];
    ParseRippleEffect(effects, config);
    ParseTrailEffect(effects, config);
    ParseIconEffect(effects, config);
    ParseTextEffect(effects, config);
}

} // namespace mousefx::config_json::parse_internal
