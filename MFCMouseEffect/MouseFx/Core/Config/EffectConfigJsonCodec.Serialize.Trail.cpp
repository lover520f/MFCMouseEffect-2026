#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildTrailProfilesJson(const TrailProfilesConfig& profiles) {
    nlohmann::json trailProfiles;
    auto addProfile = [&](const char* key, TrailHistoryProfile profile) {
        profile = config_internal::SanitizeTrailHistoryProfile(profile);
        trailProfiles[key] = {
            {keys::profile::kDurationMs, profile.durationMs},
            {keys::profile::kMaxPoints, profile.maxPoints}
        };
    };

    addProfile(keys::profile::kLine, profiles.line);
    addProfile(keys::profile::kStreamer, profiles.streamer);
    addProfile(keys::profile::kElectric, profiles.electric);
    addProfile(keys::profile::kMeteor, profiles.meteor);
    addProfile(keys::profile::kTubes, profiles.tubes);
    return trailProfiles;
}

nlohmann::json BuildTrailParamsJson(const TrailRendererParamsConfig& source) {
    const auto params = config_internal::SanitizeTrailParams(source);

    nlohmann::json trailParams;
    trailParams[keys::trail_params::kStreamer] = {
        {keys::trail_params::streamer::kGlowWidthScale, params.streamer.glowWidthScale},
        {keys::trail_params::streamer::kCoreWidthScale, params.streamer.coreWidthScale},
        {keys::trail_params::streamer::kHeadPower, params.streamer.headPower}
    };
    trailParams[keys::trail_params::kElectric] = {
        {keys::trail_params::electric::kAmplitudeScale, params.electric.amplitudeScale},
        {keys::trail_params::electric::kForkChance, params.electric.forkChance}
    };
    trailParams[keys::trail_params::kMeteor] = {
        {keys::trail_params::meteor::kSparkRateScale, params.meteor.sparkRateScale},
        {keys::trail_params::meteor::kSparkSpeedScale, params.meteor.sparkSpeedScale}
    };

    if (params.idleFade.startMs > 0) {
        trailParams[keys::trail_params::kIdleFadeStartMs] = params.idleFade.startMs;
    }
    if (params.idleFade.endMs > 0) {
        trailParams[keys::trail_params::kIdleFadeEndMs] = params.idleFade.endMs;
    }
    return trailParams;
}

} // namespace mousefx::config_json::serialize_internal
