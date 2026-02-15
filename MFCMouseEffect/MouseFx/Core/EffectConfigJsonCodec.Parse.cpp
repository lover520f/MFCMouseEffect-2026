#include "pch.h"
#include "EffectConfigJsonCodec.h"

#include "EffectConfigInternal.h"

namespace mousefx::config_json {
namespace {

template<typename T>
T GetOr(const nlohmann::json& j, const char* key, T defaultValue) {
    if (j.contains(key) && !j[key].is_null()) {
        try {
            return j[key].get<T>();
        } catch (...) {
        }
    }
    return defaultValue;
}

Argb GetColorOr(const nlohmann::json& j, const char* key, Argb defaultValue) {
    if (j.contains(key) && j[key].is_string()) {
        return ArgbFromHex(j[key].get<std::string>());
    }
    return defaultValue;
}

bool TryUtf8ToWide(const std::string& utf8, std::wstring* out) {
    if (out == nullptr) {
        return false;
    }
    if (utf8.empty()) {
        out->clear();
        return true;
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (len <= 0) {
        return false;
    }

    std::wstring wide(static_cast<size_t>(len), L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), len) <= 0) {
        return false;
    }

    if (!wide.empty() && wide.back() == L'\0') {
        wide.pop_back();
    }
    *out = wide;
    return true;
}

void ApplyCommonInputIndicatorFields(const nlohmann::json& input, InputIndicatorConfig& config) {
    config.enabled = GetOr<bool>(input, "enabled", config.enabled);
    config.keyboardEnabled = GetOr<bool>(input, "keyboard_enabled", config.keyboardEnabled);
    config.positionMode = GetOr<std::string>(input, "position_mode", config.positionMode);
    config.offsetX = GetOr<int>(input, "offset_x", config.offsetX);
    config.offsetY = GetOr<int>(input, "offset_y", config.offsetY);
    config.absoluteX = GetOr<int>(input, "absolute_x", config.absoluteX);
    config.absoluteY = GetOr<int>(input, "absolute_y", config.absoluteY);
    config.sizePx = GetOr<int>(input, "size_px", config.sizePx);
    config.durationMs = GetOr<int>(input, "duration_ms", config.durationMs);
}

void ParseInputIndicator(const nlohmann::json& root, EffectConfig& config) {
    if (root.contains("input_indicator") && root["input_indicator"].is_object()) {
        const auto& input = root["input_indicator"];
        ApplyCommonInputIndicatorFields(input, config.inputIndicator);
        config.inputIndicator.targetMonitor = GetOr<std::string>(input, "target_monitor", config.inputIndicator.targetMonitor);
        config.inputIndicator.keyDisplayMode = GetOr<std::string>(input, "key_display_mode", config.inputIndicator.keyDisplayMode);

        if (input.contains("per_monitor_overrides") && input["per_monitor_overrides"].is_object()) {
            config.inputIndicator.perMonitorOverrides.clear();
            for (auto& [key, value] : input["per_monitor_overrides"].items()) {
                if (!value.is_object()) {
                    continue;
                }

                PerMonitorPosOverride overrideConfig;
                overrideConfig.enabled = GetOr<bool>(value, "enabled", false);
                overrideConfig.absoluteX = GetOr<int>(value, "absolute_x", 40);
                overrideConfig.absoluteY = GetOr<int>(value, "absolute_y", 40);
                config.inputIndicator.perMonitorOverrides[key] = overrideConfig;
            }
        }

        config.inputIndicator = config_internal::SanitizeInputIndicatorConfig(config.inputIndicator);
        return;
    }

    if (root.contains("mouse_indicator") && root["mouse_indicator"].is_object()) {
        const auto& legacy = root["mouse_indicator"];
        ApplyCommonInputIndicatorFields(legacy, config.inputIndicator);
        config.inputIndicator = config_internal::SanitizeInputIndicatorConfig(config.inputIndicator);
    }
}

void ParseTrailParams(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains("trail_params") || !root["trail_params"].is_object()) {
        return;
    }

    const auto& trailParams = root["trail_params"];
    if (trailParams.contains("streamer") && trailParams["streamer"].is_object()) {
        const auto& streamer = trailParams["streamer"];
        config.trailParams.streamer.glowWidthScale = GetOr<float>(streamer, "glow_width_scale", config.trailParams.streamer.glowWidthScale);
        config.trailParams.streamer.coreWidthScale = GetOr<float>(streamer, "core_width_scale", config.trailParams.streamer.coreWidthScale);
        config.trailParams.streamer.headPower = GetOr<float>(streamer, "head_power", config.trailParams.streamer.headPower);
    }
    if (trailParams.contains("electric") && trailParams["electric"].is_object()) {
        const auto& electric = trailParams["electric"];
        config.trailParams.electric.amplitudeScale = GetOr<float>(electric, "amplitude_scale", config.trailParams.electric.amplitudeScale);
        config.trailParams.electric.forkChance = GetOr<float>(electric, "fork_chance", config.trailParams.electric.forkChance);
    }
    if (trailParams.contains("meteor") && trailParams["meteor"].is_object()) {
        const auto& meteor = trailParams["meteor"];
        config.trailParams.meteor.sparkRateScale = GetOr<float>(meteor, "spark_rate_scale", config.trailParams.meteor.sparkRateScale);
        config.trailParams.meteor.sparkSpeedScale = GetOr<float>(meteor, "spark_speed_scale", config.trailParams.meteor.sparkSpeedScale);
    }

    config.trailParams.idleFade.startMs = GetOr<int>(trailParams, "idle_fade_start_ms", config.trailParams.idleFade.startMs);
    config.trailParams.idleFade.endMs = GetOr<int>(trailParams, "idle_fade_end_ms", config.trailParams.idleFade.endMs);
    config.trailParams = config_internal::SanitizeTrailParams(config.trailParams);
}

void ParseTrailProfiles(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains("trail_profiles") || !root["trail_profiles"].is_object()) {
        return;
    }

    const auto& trailProfiles = root["trail_profiles"];
    auto parseProfile = [&](const char* key, TrailHistoryProfile& profile) {
        if (!trailProfiles.contains(key) || !trailProfiles[key].is_object()) {
            return;
        }

        const auto& section = trailProfiles[key];
        profile.durationMs = GetOr<int>(section, "duration_ms", profile.durationMs);
        profile.maxPoints = GetOr<int>(section, "max_points", profile.maxPoints);
        profile = config_internal::SanitizeTrailHistoryProfile(profile);
    };

    parseProfile("line", config.trailProfiles.line);
    parseProfile("streamer", config.trailProfiles.streamer);
    parseProfile("electric", config.trailProfiles.electric);
    parseProfile("meteor", config.trailProfiles.meteor);
    parseProfile("tubes", config.trailProfiles.tubes);
}

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

} // namespace

void ApplyRootToConfig(const nlohmann::json& root, EffectConfig& config) {
    config.defaultEffect = GetOr<std::string>(root, "default_effect", config.defaultEffect);
    config.theme = GetOr<std::string>(root, "theme", config.theme);
    config.uiLanguage = GetOr<std::string>(root, "ui_language", config.uiLanguage);
    config.holdFollowMode = config_internal::NormalizeHoldFollowMode(
        GetOr<std::string>(root, "hold_follow_mode", config.holdFollowMode));
    config.trailStyle = GetOr<std::string>(root, "trail_style", config.trailStyle);

    if (root.contains("active_effects") && root["active_effects"].is_object()) {
        const auto& active = root["active_effects"];
        config.active.click = GetOr<std::string>(active, "click", config.active.click);
        config.active.trail = GetOr<std::string>(active, "trail", config.active.trail);
        config.active.scroll = GetOr<std::string>(active, "scroll", config.active.scroll);
        config.active.hover = GetOr<std::string>(active, "hover", config.active.hover);
        config.active.hold = GetOr<std::string>(active, "hold", config.active.hold);
    }

    ParseInputIndicator(root, config);
    ParseTrailParams(root, config);
    ParseTrailProfiles(root, config);
    ParseEffects(root, config);
}

} // namespace mousefx::config_json
