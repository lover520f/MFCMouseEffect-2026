#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildInputIndicatorJson(const InputIndicatorConfig& source) {
    const auto input = config_internal::SanitizeInputIndicatorConfig(source);

    nlohmann::json perMonitorOverrides = nlohmann::json::object();
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

nlohmann::json BuildTrailProfilesJson(const TrailProfilesConfig& profiles) {
    nlohmann::json trailProfiles;
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

nlohmann::json BuildTrailParamsJson(const TrailRendererParamsConfig& source) {
    const auto params = config_internal::SanitizeTrailParams(source);

    nlohmann::json trailParams;
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

} // namespace mousefx::config_json::serialize_internal
