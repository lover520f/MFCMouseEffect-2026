#include "pch.h"
#include "EffectConfigJsonCodec.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

namespace mousefx::config_json {

nlohmann::json BuildRootFromConfig(const EffectConfig& config) {
    nlohmann::json root;
    root[keys::kDefaultEffect] = config.defaultEffect;
    root[keys::kTheme] = config.theme;
    root[keys::kThemeCatalogRootPath] = config.themeCatalogRootPath;
    root[keys::kOverlayTargetFps] = config_internal::SanitizeOverlayTargetFps(config.overlayTargetFps);
    root[keys::kUiLanguage] = config.uiLanguage;
    root[keys::kHoldFollowMode] = config_internal::NormalizeHoldFollowMode(config.holdFollowMode);
    root[keys::kHoldPresenterBackend] = config_internal::NormalizeHoldPresenterBackend(config.holdPresenterBackend);
    root[keys::kTrailStyle] = config.trailStyle;
    root[keys::kInputIndicator] = serialize_internal::BuildInputIndicatorJson(config.inputIndicator);
    root[keys::kAutomation] = serialize_internal::BuildAutomationJson(config.automation);
    root[keys::kWasm] = serialize_internal::BuildWasmJson(config.wasm);
    root[keys::kTrailProfiles] = serialize_internal::BuildTrailProfilesJson(config.trailProfiles);
    root[keys::kTrailParams] = serialize_internal::BuildTrailParamsJson(config.trailParams);
    root[keys::kActiveEffects] = serialize_internal::BuildActiveEffectsJson(config.active);
    root[keys::kEffects] = serialize_internal::BuildEffectsJson(config);
    return root;
}

} // namespace mousefx::config_json
