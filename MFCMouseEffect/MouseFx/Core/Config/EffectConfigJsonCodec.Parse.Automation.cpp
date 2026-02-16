#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::parse_internal {
namespace {

void ParseBindingsArray(
    const nlohmann::json& source,
    std::vector<AutomationKeyBinding>* outBindings) {
    if (!outBindings || !source.is_array()) {
        return;
    }

    outBindings->clear();
    for (const auto& item : source) {
        if (!item.is_object()) {
            continue;
        }
        AutomationKeyBinding binding;
        binding.enabled = GetOr<bool>(item, keys::automation::kEnabled, binding.enabled);
        binding.trigger = GetOr<std::string>(item, keys::automation::kTrigger, binding.trigger);
        binding.keys = GetOr<std::string>(item, keys::automation::kKeys, binding.keys);
        outBindings->push_back(binding);
    }
}

} // namespace

void ParseAutomation(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains(keys::kAutomation) || !root[keys::kAutomation].is_object()) {
        return;
    }

    const auto& automation = root[keys::kAutomation];
    config.automation.enabled = GetOr<bool>(automation, keys::automation::kEnabled, config.automation.enabled);

    if (automation.contains(keys::automation::kMouseMappings)) {
        ParseBindingsArray(automation[keys::automation::kMouseMappings], &config.automation.mouseMappings);
    }

    if (automation.contains(keys::automation::kGesture) &&
        automation[keys::automation::kGesture].is_object()) {
        const auto& gesture = automation[keys::automation::kGesture];
        config.automation.gesture.enabled =
            GetOr<bool>(gesture, keys::automation::kEnabled, config.automation.gesture.enabled);
        config.automation.gesture.triggerButton =
            GetOr<std::string>(gesture, keys::automation::kTriggerButton, config.automation.gesture.triggerButton);
        config.automation.gesture.minStrokeDistancePx =
            GetOr<int>(gesture, keys::automation::kMinStrokeDistancePx, config.automation.gesture.minStrokeDistancePx);
        config.automation.gesture.sampleStepPx =
            GetOr<int>(gesture, keys::automation::kSampleStepPx, config.automation.gesture.sampleStepPx);
        config.automation.gesture.maxDirections =
            GetOr<int>(gesture, keys::automation::kMaxDirections, config.automation.gesture.maxDirections);

        if (gesture.contains(keys::automation::kMappings)) {
            ParseBindingsArray(gesture[keys::automation::kMappings], &config.automation.gesture.mappings);
        }
    }

    config.automation = config_internal::SanitizeInputAutomationConfig(config.automation);
}

} // namespace mousefx::config_json::parse_internal
