#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::parse_internal {
namespace {

void ApplyCommonInputIndicatorFields(const nlohmann::json& input, InputIndicatorConfig& config) {
    config.enabled = GetOr<bool>(input, keys::input::kEnabled, config.enabled);
    config.keyboardEnabled = GetOr<bool>(input, keys::input::kKeyboardEnabled, config.keyboardEnabled);
    config.positionMode = GetOr<std::string>(input, keys::input::kPositionMode, config.positionMode);
    config.offsetX = GetOr<int>(input, keys::input::kOffsetX, config.offsetX);
    config.offsetY = GetOr<int>(input, keys::input::kOffsetY, config.offsetY);
    config.absoluteX = GetOr<int>(input, keys::input::kAbsoluteX, config.absoluteX);
    config.absoluteY = GetOr<int>(input, keys::input::kAbsoluteY, config.absoluteY);
    config.sizePx = GetOr<int>(input, keys::input::kSizePx, config.sizePx);
    config.durationMs = GetOr<int>(input, keys::input::kDurationMs, config.durationMs);
}

} // namespace

void ParseInputIndicator(const nlohmann::json& root, EffectConfig& config) {
    if (root.contains(keys::kInputIndicator) && root[keys::kInputIndicator].is_object()) {
        const auto& input = root[keys::kInputIndicator];
        ApplyCommonInputIndicatorFields(input, config.inputIndicator);
        config.inputIndicator.targetMonitor = GetOr<std::string>(input, keys::input::kTargetMonitor, config.inputIndicator.targetMonitor);
        config.inputIndicator.keyDisplayMode = GetOr<std::string>(input, keys::input::kKeyDisplayMode, config.inputIndicator.keyDisplayMode);

        if (input.contains(keys::input::kPerMonitorOverrides) && input[keys::input::kPerMonitorOverrides].is_object()) {
            config.inputIndicator.perMonitorOverrides.clear();
            for (auto& [key, value] : input[keys::input::kPerMonitorOverrides].items()) {
                if (!value.is_object()) {
                    continue;
                }

                PerMonitorPosOverride overrideConfig;
                overrideConfig.enabled = GetOr<bool>(value, keys::input::kEnabled, false);
                overrideConfig.absoluteX = GetOr<int>(value, keys::input::kAbsoluteX, 40);
                overrideConfig.absoluteY = GetOr<int>(value, keys::input::kAbsoluteY, 40);
                config.inputIndicator.perMonitorOverrides[key] = overrideConfig;
            }
        }

        config.inputIndicator = config_internal::SanitizeInputIndicatorConfig(config.inputIndicator);
        return;
    }

    if (root.contains(keys::kMouseIndicator) && root[keys::kMouseIndicator].is_object()) {
        const auto& legacy = root[keys::kMouseIndicator];
        ApplyCommonInputIndicatorFields(legacy, config.inputIndicator);
        config.inputIndicator = config_internal::SanitizeInputIndicatorConfig(config.inputIndicator);
    }
}

void ParseTrailParams(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains(keys::kTrailParams) || !root[keys::kTrailParams].is_object()) {
        return;
    }

    const auto& trailParams = root[keys::kTrailParams];
    if (trailParams.contains(keys::trail_params::kStreamer) && trailParams[keys::trail_params::kStreamer].is_object()) {
        const auto& streamer = trailParams[keys::trail_params::kStreamer];
        config.trailParams.streamer.glowWidthScale = GetOr<float>(streamer, keys::trail_params::streamer::kGlowWidthScale, config.trailParams.streamer.glowWidthScale);
        config.trailParams.streamer.coreWidthScale = GetOr<float>(streamer, keys::trail_params::streamer::kCoreWidthScale, config.trailParams.streamer.coreWidthScale);
        config.trailParams.streamer.headPower = GetOr<float>(streamer, keys::trail_params::streamer::kHeadPower, config.trailParams.streamer.headPower);
    }
    if (trailParams.contains(keys::trail_params::kElectric) && trailParams[keys::trail_params::kElectric].is_object()) {
        const auto& electric = trailParams[keys::trail_params::kElectric];
        config.trailParams.electric.amplitudeScale = GetOr<float>(electric, keys::trail_params::electric::kAmplitudeScale, config.trailParams.electric.amplitudeScale);
        config.trailParams.electric.forkChance = GetOr<float>(electric, keys::trail_params::electric::kForkChance, config.trailParams.electric.forkChance);
    }
    if (trailParams.contains(keys::trail_params::kMeteor) && trailParams[keys::trail_params::kMeteor].is_object()) {
        const auto& meteor = trailParams[keys::trail_params::kMeteor];
        config.trailParams.meteor.sparkRateScale = GetOr<float>(meteor, keys::trail_params::meteor::kSparkRateScale, config.trailParams.meteor.sparkRateScale);
        config.trailParams.meteor.sparkSpeedScale = GetOr<float>(meteor, keys::trail_params::meteor::kSparkSpeedScale, config.trailParams.meteor.sparkSpeedScale);
    }

    config.trailParams.idleFade.startMs = GetOr<int>(trailParams, keys::trail_params::kIdleFadeStartMs, config.trailParams.idleFade.startMs);
    config.trailParams.idleFade.endMs = GetOr<int>(trailParams, keys::trail_params::kIdleFadeEndMs, config.trailParams.idleFade.endMs);
    config.trailParams = config_internal::SanitizeTrailParams(config.trailParams);
}

void ParseTrailProfiles(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains(keys::kTrailProfiles) || !root[keys::kTrailProfiles].is_object()) {
        return;
    }

    const auto& trailProfiles = root[keys::kTrailProfiles];
    auto parseProfile = [&](const char* key, TrailHistoryProfile& profile) {
        if (!trailProfiles.contains(key) || !trailProfiles[key].is_object()) {
            return;
        }

        const auto& section = trailProfiles[key];
        profile.durationMs = GetOr<int>(section, keys::profile::kDurationMs, profile.durationMs);
        profile.maxPoints = GetOr<int>(section, keys::profile::kMaxPoints, profile.maxPoints);
        profile = config_internal::SanitizeTrailHistoryProfile(profile);
    };

    parseProfile(keys::profile::kLine, config.trailProfiles.line);
    parseProfile(keys::profile::kStreamer, config.trailProfiles.streamer);
    parseProfile(keys::profile::kElectric, config.trailProfiles.electric);
    parseProfile(keys::profile::kMeteor, config.trailProfiles.meteor);
    parseProfile(keys::profile::kTubes, config.trailProfiles.tubes);
}

} // namespace mousefx::config_json::parse_internal
