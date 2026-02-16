#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::serialize_internal {
namespace {

nlohmann::json BuildBindingsArray(const std::vector<AutomationKeyBinding>& bindings) {
    nlohmann::json out = nlohmann::json::array();
    for (const AutomationKeyBinding& binding : bindings) {
        out.push_back({
            {keys::automation::kEnabled, binding.enabled},
            {keys::automation::kTrigger, binding.trigger},
            {keys::automation::kKeys, binding.keys},
        });
    }
    return out;
}

} // namespace

nlohmann::json BuildAutomationJson(const InputAutomationConfig& source) {
    const InputAutomationConfig config = config_internal::SanitizeInputAutomationConfig(source);
    return {
        {keys::automation::kEnabled, config.enabled},
        {keys::automation::kMouseMappings, BuildBindingsArray(config.mouseMappings)},
        {keys::automation::kGesture, {
            {keys::automation::kEnabled, config.gesture.enabled},
            {keys::automation::kTriggerButton, config.gesture.triggerButton},
            {keys::automation::kMinStrokeDistancePx, config.gesture.minStrokeDistancePx},
            {keys::automation::kSampleStepPx, config.gesture.sampleStepPx},
            {keys::automation::kMaxDirections, config.gesture.maxDirections},
            {keys::automation::kMappings, BuildBindingsArray(config.gesture.mappings)},
        }},
    };
}

} // namespace mousefx::config_json::serialize_internal
