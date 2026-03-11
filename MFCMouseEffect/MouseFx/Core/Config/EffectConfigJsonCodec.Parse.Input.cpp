#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::parse_internal {
namespace {

void ApplyCommonInputIndicatorFields(const nlohmann::json& input, InputIndicatorConfig& config) {
    config.enabled = GetOr<bool>(input, keys::input::kEnabled, config.enabled);
    config.keyboardEnabled = GetOr<bool>(input, keys::input::kKeyboardEnabled, config.keyboardEnabled);
    config.renderMode = GetOr<std::string>(input, keys::input::kRenderMode, config.renderMode);
    config.wasmFallbackToNative =
        GetOr<bool>(input, keys::input::kWasmFallbackToNative, config.wasmFallbackToNative);
    config.wasmManifestPath =
        GetOr<std::string>(input, keys::input::kWasmManifestPath, config.wasmManifestPath);
    config.positionMode = GetOr<std::string>(input, keys::input::kPositionMode, config.positionMode);
    config.offsetX = GetOr<int>(input, keys::input::kOffsetX, config.offsetX);
    config.offsetY = GetOr<int>(input, keys::input::kOffsetY, config.offsetY);
    config.absoluteX = GetOr<int>(input, keys::input::kAbsoluteX, config.absoluteX);
    config.absoluteY = GetOr<int>(input, keys::input::kAbsoluteY, config.absoluteY);
    config.keyLabelLayoutMode =
        GetOr<std::string>(input, keys::input::kKeyLabelLayoutMode, config.keyLabelLayoutMode);
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

} // namespace mousefx::config_json::parse_internal
