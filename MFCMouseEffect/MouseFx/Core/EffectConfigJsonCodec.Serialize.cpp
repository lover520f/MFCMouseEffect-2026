#include "pch.h"
#include "EffectConfigJsonCodec.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

namespace mousefx::config_json {

nlohmann::json BuildRootFromConfig(const EffectConfig& config) {
    nlohmann::json root;
    root["default_effect"] = config.defaultEffect;
    root["theme"] = config.theme;
    root["ui_language"] = config.uiLanguage;
    root["hold_follow_mode"] = config_internal::NormalizeHoldFollowMode(config.holdFollowMode);
    root["trail_style"] = config.trailStyle;
    root["input_indicator"] = serialize_internal::BuildInputIndicatorJson(config.inputIndicator);
    root["trail_profiles"] = serialize_internal::BuildTrailProfilesJson(config.trailProfiles);
    root["trail_params"] = serialize_internal::BuildTrailParamsJson(config.trailParams);
    root["active_effects"] = serialize_internal::BuildActiveEffectsJson(config.active);
    root["effects"] = serialize_internal::BuildEffectsJson(config);
    return root;
}

} // namespace mousefx::config_json
