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
        binding.triggerButton = GetOr<std::string>(item, keys::automation::kTriggerButton, binding.triggerButton);
        binding.appScopes.clear();
        if (item.contains(keys::automation::kAppScopes) &&
            item[keys::automation::kAppScopes].is_array()) {
            for (const auto& scope : item[keys::automation::kAppScopes]) {
                if (scope.is_string()) {
                    binding.appScopes.push_back(scope.get<std::string>());
                }
            }
        }
        if (binding.appScopes.empty() &&
            item.contains(keys::automation::kAppScope) &&
            item[keys::automation::kAppScope].is_string()) {
            binding.appScopes.push_back(item[keys::automation::kAppScope].get<std::string>());
        }
        if (item.contains(keys::automation::kGesturePattern) &&
            item[keys::automation::kGesturePattern].is_object()) {
            const auto& gesturePattern = item[keys::automation::kGesturePattern];
            binding.gesturePattern.mode = GetOr<std::string>(
                gesturePattern,
                keys::automation::kGesturePatternMode,
                binding.gesturePattern.mode);
            binding.gesturePattern.matchThresholdPercent = GetOr<int>(
                gesturePattern,
                keys::automation::kGestureMatchThresholdPercent,
                binding.gesturePattern.matchThresholdPercent);
            binding.gesturePattern.customPoints.clear();
            binding.gesturePattern.customStrokes.clear();
            if (gesturePattern.contains(keys::automation::kGestureCustomPoints) &&
                gesturePattern[keys::automation::kGestureCustomPoints].is_array()) {
                for (const auto& point : gesturePattern[keys::automation::kGestureCustomPoints]) {
                    if (!point.is_object()) {
                        continue;
                    }
                    AutomationKeyBinding::GesturePoint gesturePoint;
                    gesturePoint.x = GetOr<int>(point, keys::automation::kPointX, gesturePoint.x);
                    gesturePoint.y = GetOr<int>(point, keys::automation::kPointY, gesturePoint.y);
                    binding.gesturePattern.customPoints.push_back(gesturePoint);
                }
            }
            if (gesturePattern.contains(keys::automation::kGestureCustomStrokes) &&
                gesturePattern[keys::automation::kGestureCustomStrokes].is_array()) {
                for (const auto& stroke : gesturePattern[keys::automation::kGestureCustomStrokes]) {
                    if (!stroke.is_array()) {
                        continue;
                    }
                    std::vector<AutomationKeyBinding::GesturePoint> strokePoints;
                    for (const auto& point : stroke) {
                        if (!point.is_object()) {
                            continue;
                        }
                        AutomationKeyBinding::GesturePoint gesturePoint;
                        gesturePoint.x = GetOr<int>(point, keys::automation::kPointX, gesturePoint.x);
                        gesturePoint.y = GetOr<int>(point, keys::automation::kPointY, gesturePoint.y);
                        strokePoints.push_back(gesturePoint);
                    }
                    if (!strokePoints.empty()) {
                        binding.gesturePattern.customStrokes.push_back(std::move(strokePoints));
                    }
                }
            }
        }
        if (item.contains(keys::automation::kModifiers) &&
            item[keys::automation::kModifiers].is_object()) {
            const auto& modifiers = item[keys::automation::kModifiers];
            binding.modifiers.mode = GetOr<std::string>(
                modifiers,
                keys::automation::kModifierMode,
                binding.modifiers.mode);
            binding.modifiers.primary = GetOr<bool>(
                modifiers,
                keys::automation::kModifierPrimary,
                binding.modifiers.primary);
            binding.modifiers.shift = GetOr<bool>(
                modifiers,
                keys::automation::kModifierShift,
                binding.modifiers.shift);
            binding.modifiers.alt = GetOr<bool>(
                modifiers,
                keys::automation::kModifierAlt,
                binding.modifiers.alt);
        }
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
