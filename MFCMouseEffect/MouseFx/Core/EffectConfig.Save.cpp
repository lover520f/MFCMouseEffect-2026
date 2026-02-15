#include "pch.h"
#include "EffectConfig.h"
#include "EffectConfigInternal.h"

#include "MouseFx/ThirdParty/json.hpp"

#include <fstream>

using json = nlohmann::json;

namespace mousefx {
namespace {

json BuildInputIndicatorJson(const InputIndicatorConfig& source) {
    const auto input = config_internal::SanitizeInputIndicatorConfig(source);

    json perMonitorOverrides = json::object();
    for (const auto& [key, value] : input.perMonitorOverrides) {
        perMonitorOverrides[key] = {
            {"enabled", value.enabled},
            {"absolute_x", value.absoluteX},
            {"absolute_y", value.absoluteY}
        };
    }

    return {
        {"enabled", input.enabled},
        {"keyboard_enabled", input.keyboardEnabled},
        {"position_mode", input.positionMode},
        {"offset_x", input.offsetX},
        {"offset_y", input.offsetY},
        {"absolute_x", input.absoluteX},
        {"absolute_y", input.absoluteY},
        {"target_monitor", input.targetMonitor},
        {"key_display_mode", input.keyDisplayMode},
        {"size_px", input.sizePx},
        {"duration_ms", input.durationMs},
        {"per_monitor_overrides", perMonitorOverrides}
    };
}

json BuildTrailProfilesJson(const TrailProfilesConfig& profiles) {
    json trailProfiles;
    auto addProfile = [&](const char* key, TrailHistoryProfile profile) {
        profile = config_internal::SanitizeTrailHistoryProfile(profile);
        trailProfiles[key] = {
            {"duration_ms", profile.durationMs},
            {"max_points", profile.maxPoints}
        };
    };

    addProfile("line", profiles.line);
    addProfile("streamer", profiles.streamer);
    addProfile("electric", profiles.electric);
    addProfile("meteor", profiles.meteor);
    addProfile("tubes", profiles.tubes);
    return trailProfiles;
}

json BuildTrailParamsJson(const TrailRendererParamsConfig& source) {
    const auto params = config_internal::SanitizeTrailParams(source);

    json trailParams;
    trailParams["streamer"] = {
        {"glow_width_scale", params.streamer.glowWidthScale},
        {"core_width_scale", params.streamer.coreWidthScale},
        {"head_power", params.streamer.headPower}
    };
    trailParams["electric"] = {
        {"amplitude_scale", params.electric.amplitudeScale},
        {"fork_chance", params.electric.forkChance}
    };
    trailParams["meteor"] = {
        {"spark_rate_scale", params.meteor.sparkRateScale},
        {"spark_speed_scale", params.meteor.sparkSpeedScale}
    };

    if (params.idleFade.startMs > 0) {
        trailParams["idle_fade_start_ms"] = params.idleFade.startMs;
    }
    if (params.idleFade.endMs > 0) {
        trailParams["idle_fade_end_ms"] = params.idleFade.endMs;
    }
    return trailParams;
}

json BuildActiveEffectsJson(const ActiveEffectConfig& active) {
    return {
        {"click", active.click},
        {"trail", active.trail},
        {"scroll", active.scroll},
        {"hover", active.hover},
        {"hold", active.hold}
    };
}

json BuildEffectsJson(const EffectConfig& config) {
    json effects;

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

    json textClick;
    textClick["duration_ms"] = config.textClick.durationMs;
    textClick["float_distance"] = config.textClick.floatDistance;
    textClick["font_size"] = config.textClick.fontSize;
    textClick["font_family"] = config_internal::WStringToUtf8(config.textClick.fontFamily);

    json texts = json::array();
    for (const auto& text : config.textClick.texts) {
        texts.push_back(config_internal::WStringToUtf8(text));
    }
    textClick["texts"] = texts;

    json colors = json::array();
    for (const auto& color : config.textClick.colors) {
        colors.push_back(config_internal::ArgbToHex(color));
    }
    textClick["colors"] = colors;
    effects["text_click"] = textClick;

    return effects;
}

} // namespace

bool EffectConfig::Save(const std::wstring& exeDir, const EffectConfig& config) {
    std::wstring configPath = exeDir + L"\\config.json";

    json root;
    root["default_effect"] = config.defaultEffect;
    root["theme"] = config.theme;
    root["ui_language"] = config.uiLanguage;
    root["hold_follow_mode"] = config_internal::NormalizeHoldFollowMode(config.holdFollowMode);
    root["trail_style"] = config.trailStyle;
    root["input_indicator"] = BuildInputIndicatorJson(config.inputIndicator);
    root["trail_profiles"] = BuildTrailProfilesJson(config.trailProfiles);
    root["trail_params"] = BuildTrailParamsJson(config.trailParams);
    root["active_effects"] = BuildActiveEffectsJson(config.active);
    root["effects"] = BuildEffectsJson(config);

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }

    file << root.dump(2);
    return true;
}

} // namespace mousefx
