#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

#include "EffectConfigInternal.h"

namespace mousefx::config_json::parse_internal {
namespace {

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

} // namespace

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

} // namespace mousefx::config_json::parse_internal
