#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::serialize_internal {
namespace {

nlohmann::json BuildBindingsArray(const std::vector<AutomationKeyBinding>& bindings, bool gestureBindings) {
    nlohmann::json out = nlohmann::json::array();
    for (const AutomationKeyBinding& binding : bindings) {
        const std::vector<std::string> appScopes =
            binding.appScopes.empty() ? std::vector<std::string>{"all"} : binding.appScopes;
        const std::string legacyAppScope = appScopes.front();
        const std::vector<std::vector<AutomationKeyBinding::GesturePoint>> strokeSource =
            !binding.gesturePattern.customStrokes.empty()
            ? binding.gesturePattern.customStrokes
            : std::vector<std::vector<AutomationKeyBinding::GesturePoint>>{
                binding.gesturePattern.customPoints.empty()
                    ? std::vector<AutomationKeyBinding::GesturePoint>{}
                    : binding.gesturePattern.customPoints,
            };
        nlohmann::json item = {
            {keys::automation::kEnabled, binding.enabled},
            {keys::automation::kTrigger, binding.trigger},
            {keys::automation::kAppScope, legacyAppScope},
            {keys::automation::kAppScopes, appScopes},
            {keys::automation::kGesturePattern, {
                {keys::automation::kGesturePatternMode, binding.gesturePattern.mode},
                {keys::automation::kGestureMatchThresholdPercent, binding.gesturePattern.matchThresholdPercent},
                {keys::automation::kGestureCustomPoints, [&]() {
                    nlohmann::json points = nlohmann::json::array();
                    for (const auto& point : binding.gesturePattern.customPoints) {
                        points.push_back({
                            {keys::automation::kPointX, point.x},
                            {keys::automation::kPointY, point.y},
                        });
                    }
                    return points;
                }()},
                {keys::automation::kGestureCustomStrokes, [&]() {
                    nlohmann::json strokes = nlohmann::json::array();
                    for (const auto& stroke : strokeSource) {
                        if (stroke.empty()) {
                            continue;
                        }
                        nlohmann::json points = nlohmann::json::array();
                        for (const auto& point : stroke) {
                            points.push_back({
                                {keys::automation::kPointX, point.x},
                                {keys::automation::kPointY, point.y},
                            });
                        }
                        strokes.push_back(std::move(points));
                    }
                    return strokes;
                }()},
            }},
            {keys::automation::kModifiers, {
                {keys::automation::kModifierMode, binding.modifiers.mode},
                {keys::automation::kModifierPrimary, binding.modifiers.primary},
                {keys::automation::kModifierShift, binding.modifiers.shift},
                {keys::automation::kModifierAlt, binding.modifiers.alt},
            }},
            {keys::automation::kKeys, binding.keys},
        };
        if (gestureBindings) {
            item[keys::automation::kTriggerButton] = binding.triggerButton;
        }
        out.push_back(std::move(item));
    }
    return out;
}

} // namespace

nlohmann::json BuildAutomationJson(const InputAutomationConfig& source) {
    const InputAutomationConfig config = config_internal::SanitizeInputAutomationConfig(source);
    return {
        {keys::automation::kEnabled, config.enabled},
        {keys::automation::kMouseMappings, BuildBindingsArray(config.mouseMappings, false)},
        {keys::automation::kGesture, {
            {keys::automation::kEnabled, config.gesture.enabled},
            {keys::automation::kTriggerButton, config.gesture.triggerButton},
            {keys::automation::kMinStrokeDistancePx, config.gesture.minStrokeDistancePx},
            {keys::automation::kSampleStepPx, config.gesture.sampleStepPx},
            {keys::automation::kMaxDirections, config.gesture.maxDirections},
            {keys::automation::kMappings, BuildBindingsArray(config.gesture.mappings, true)},
        }},
    };
}

} // namespace mousefx::config_json::serialize_internal
