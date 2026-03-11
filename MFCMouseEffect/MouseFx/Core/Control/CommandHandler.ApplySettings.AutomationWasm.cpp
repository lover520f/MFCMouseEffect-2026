#include "pch.h"

#include "CommandHandler.ApplySettings.Internal.h"

#include "AppController.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"

#include <limits>

namespace mousefx::command_handler_apply_settings {
namespace {

void ApplyAutomationBindings(const json& source, std::vector<AutomationKeyBinding>* outBindings) {
    if (!outBindings || !source.is_array()) {
        return;
    }

    outBindings->clear();
    for (const auto& item : source) {
        if (!item.is_object()) {
            continue;
        }
        AutomationKeyBinding binding;
        if (item.contains("enabled") && item["enabled"].is_boolean()) {
            binding.enabled = item["enabled"].get<bool>();
        }
        if (item.contains("trigger") && item["trigger"].is_string()) {
            binding.trigger = item["trigger"].get<std::string>();
        }
        if (item.contains("trigger_button") && item["trigger_button"].is_string()) {
            binding.triggerButton = item["trigger_button"].get<std::string>();
        }
        binding.appScopes.clear();
        if (item.contains("app_scopes") && item["app_scopes"].is_array()) {
            for (const auto& scope : item["app_scopes"]) {
                if (scope.is_string()) {
                    binding.appScopes.push_back(scope.get<std::string>());
                }
            }
        }
        if (binding.appScopes.empty() &&
            item.contains("app_scope") &&
            item["app_scope"].is_string()) {
            binding.appScopes.push_back(item["app_scope"].get<std::string>());
        }
        if (item.contains("gesture_pattern") && item["gesture_pattern"].is_object()) {
            const auto& gesturePattern = item["gesture_pattern"];
            binding.gesturePattern.mode = gesturePattern.value("mode", binding.gesturePattern.mode);
            if (gesturePattern.contains("match_threshold_percent") &&
                gesturePattern["match_threshold_percent"].is_number_integer()) {
                binding.gesturePattern.matchThresholdPercent = gesturePattern["match_threshold_percent"].get<int>();
            } else if (gesturePattern.contains("matchThresholdPercent") &&
                       gesturePattern["matchThresholdPercent"].is_number_integer()) {
                binding.gesturePattern.matchThresholdPercent = gesturePattern["matchThresholdPercent"].get<int>();
            }
            binding.gesturePattern.customPoints.clear();
            binding.gesturePattern.customStrokes.clear();
            if (gesturePattern.contains("custom_points") &&
                gesturePattern["custom_points"].is_array()) {
                for (const auto& point : gesturePattern["custom_points"]) {
                    if (!point.is_object()) {
                        continue;
                    }
                    AutomationKeyBinding::GesturePoint gesturePoint;
                    if (point.contains("x") && point["x"].is_number_integer()) {
                        gesturePoint.x = point["x"].get<int>();
                    }
                    if (point.contains("y") && point["y"].is_number_integer()) {
                        gesturePoint.y = point["y"].get<int>();
                    }
                    binding.gesturePattern.customPoints.push_back(gesturePoint);
                }
            } else if (gesturePattern.contains("customPoints") &&
                       gesturePattern["customPoints"].is_array()) {
                for (const auto& point : gesturePattern["customPoints"]) {
                    if (!point.is_object()) {
                        continue;
                    }
                    AutomationKeyBinding::GesturePoint gesturePoint;
                    if (point.contains("x") && point["x"].is_number_integer()) {
                        gesturePoint.x = point["x"].get<int>();
                    }
                    if (point.contains("y") && point["y"].is_number_integer()) {
                        gesturePoint.y = point["y"].get<int>();
                    }
                    binding.gesturePattern.customPoints.push_back(gesturePoint);
                }
            }
            const auto parseStrokes = [&](const char* key) {
                if (!gesturePattern.contains(key) || !gesturePattern[key].is_array()) {
                    return;
                }
                for (const auto& stroke : gesturePattern[key]) {
                    if (!stroke.is_array()) {
                        continue;
                    }
                    std::vector<AutomationKeyBinding::GesturePoint> strokePoints;
                    for (const auto& point : stroke) {
                        if (!point.is_object()) {
                            continue;
                        }
                        AutomationKeyBinding::GesturePoint gesturePoint;
                        if (point.contains("x") && point["x"].is_number_integer()) {
                            gesturePoint.x = point["x"].get<int>();
                        }
                        if (point.contains("y") && point["y"].is_number_integer()) {
                            gesturePoint.y = point["y"].get<int>();
                        }
                        strokePoints.push_back(gesturePoint);
                    }
                    if (!strokePoints.empty()) {
                        binding.gesturePattern.customStrokes.push_back(std::move(strokePoints));
                    }
                }
            };
            parseStrokes("custom_strokes");
            parseStrokes("customStrokes");
            if (binding.gesturePattern.customStrokes.empty() &&
                !binding.gesturePattern.customPoints.empty()) {
                binding.gesturePattern.customStrokes.push_back(binding.gesturePattern.customPoints);
            }
        }
        if (item.contains("modifiers") && item["modifiers"].is_object()) {
            const auto& modifiers = item["modifiers"];
            if (modifiers.contains("mode") && modifiers["mode"].is_string()) {
                binding.modifiers.mode = modifiers["mode"].get<std::string>();
            }
            if (modifiers.contains("primary") && modifiers["primary"].is_boolean()) {
                binding.modifiers.primary = modifiers["primary"].get<bool>();
            }
            if (modifiers.contains("shift") && modifiers["shift"].is_boolean()) {
                binding.modifiers.shift = modifiers["shift"].get<bool>();
            }
            if (modifiers.contains("alt") && modifiers["alt"].is_boolean()) {
                binding.modifiers.alt = modifiers["alt"].get<bool>();
            }
        }
        if (item.contains("keys") && item["keys"].is_string()) {
            binding.keys = item["keys"].get<std::string>();
        }
        outBindings->push_back(binding);
    }
}

} // namespace

void ApplyAutomationSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("automation") || !payload["automation"].is_object()) {
        return;
    }

    InputAutomationConfig automation = controller->Config().automation;
    const json& source = payload["automation"];
    if (source.contains("enabled") && source["enabled"].is_boolean()) {
        automation.enabled = source["enabled"].get<bool>();
    }
    if (source.contains("mouse_mappings")) {
        ApplyAutomationBindings(source["mouse_mappings"], &automation.mouseMappings);
    }

    if (source.contains("gesture") && source["gesture"].is_object()) {
        const json& gesture = source["gesture"];
        if (gesture.contains("enabled") && gesture["enabled"].is_boolean()) {
            automation.gesture.enabled = gesture["enabled"].get<bool>();
        }
        if (gesture.contains("trigger_button") && gesture["trigger_button"].is_string()) {
            automation.gesture.triggerButton = gesture["trigger_button"].get<std::string>();
        }
        if (gesture.contains("min_stroke_distance_px") && gesture["min_stroke_distance_px"].is_number_integer()) {
            automation.gesture.minStrokeDistancePx = gesture["min_stroke_distance_px"].get<int>();
        }
        if (gesture.contains("sample_step_px") && gesture["sample_step_px"].is_number_integer()) {
            automation.gesture.sampleStepPx = gesture["sample_step_px"].get<int>();
        }
        if (gesture.contains("max_directions") && gesture["max_directions"].is_number_integer()) {
            automation.gesture.maxDirections = gesture["max_directions"].get<int>();
        }
        if (gesture.contains("mappings")) {
            ApplyAutomationBindings(gesture["mappings"], &automation.gesture.mappings);
        }
    }

    controller->SetInputAutomationConfig(config_internal::SanitizeInputAutomationConfig(automation));
}

void ApplyWasmSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("wasm") || !payload["wasm"].is_object()) {
        return;
    }

    const json& source = payload["wasm"];
    if (source.contains("enabled") && source["enabled"].is_boolean()) {
        controller->SetWasmEnabled(source["enabled"].get<bool>());
    }
    if (source.contains("fallback_to_builtin_click") && source["fallback_to_builtin_click"].is_boolean()) {
        controller->SetWasmFallbackToBuiltinClick(source["fallback_to_builtin_click"].get<bool>());
    }
    if (source.contains("manifest_path") && source["manifest_path"].is_string()) {
        controller->SetWasmManifestPath(source["manifest_path"].get<std::string>());
    }
    if (source.contains("catalog_root_path") && source["catalog_root_path"].is_string()) {
        controller->SetWasmCatalogRootPath(source["catalog_root_path"].get<std::string>());
    }

    bool hasOutputBudget = false;
    bool hasCommandBudget = false;
    bool hasExecutionBudget = false;
    uint32_t outputBudgetBytes = controller->Config().wasm.outputBufferBytes;
    uint32_t maxCommands = controller->Config().wasm.maxCommands;
    double maxExecutionMs = controller->Config().wasm.maxEventExecutionMs;

    if (source.contains("output_buffer_bytes") && source["output_buffer_bytes"].is_number_integer()) {
        const int64_t raw = source["output_buffer_bytes"].get<int64_t>();
        outputBudgetBytes = (raw <= 0)
            ? 0u
            : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
        hasOutputBudget = true;
    }
    if (source.contains("max_commands") && source["max_commands"].is_number_integer()) {
        const int64_t raw = source["max_commands"].get<int64_t>();
        maxCommands = (raw <= 0)
            ? 0u
            : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
        hasCommandBudget = true;
    }
    if (source.contains("max_execution_ms") && source["max_execution_ms"].is_number()) {
        maxExecutionMs = source["max_execution_ms"].get<double>();
        hasExecutionBudget = true;
    }
    if (hasOutputBudget || hasCommandBudget || hasExecutionBudget) {
        controller->SetWasmExecutionBudget(outputBudgetBytes, maxCommands, maxExecutionMs);
    }
}

} // namespace mousefx::command_handler_apply_settings
