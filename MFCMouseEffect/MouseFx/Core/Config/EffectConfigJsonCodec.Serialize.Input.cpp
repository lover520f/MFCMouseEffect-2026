#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildInputIndicatorJson(const InputIndicatorConfig& source) {
    const auto input = config_internal::SanitizeInputIndicatorConfig(source);

    nlohmann::json perMonitorOverrides = nlohmann::json::object();
    for (const auto& [key, value] : input.perMonitorOverrides) {
        perMonitorOverrides[key] = {
            {keys::input::kEnabled, value.enabled},
            {keys::input::kAbsoluteX, value.absoluteX},
            {keys::input::kAbsoluteY, value.absoluteY}
        };
    }

    return {
        {keys::input::kEnabled, input.enabled},
        {keys::input::kKeyboardEnabled, input.keyboardEnabled},
        {keys::input::kRenderMode, input.renderMode},
        {keys::input::kWasmFallbackToNative, input.wasmFallbackToNative},
        {keys::input::kWasmManifestPath, input.wasmManifestPath},
        {keys::input::kPositionMode, input.positionMode},
        {keys::input::kOffsetX, input.offsetX},
        {keys::input::kOffsetY, input.offsetY},
        {keys::input::kAbsoluteX, input.absoluteX},
        {keys::input::kAbsoluteY, input.absoluteY},
        {keys::input::kTargetMonitor, input.targetMonitor},
        {keys::input::kKeyDisplayMode, input.keyDisplayMode},
        {keys::input::kKeyLabelLayoutMode, input.keyLabelLayoutMode},
        {keys::input::kSizePx, input.sizePx},
        {keys::input::kDurationMs, input.durationMs},
        {keys::input::kPerMonitorOverrides, perMonitorOverrides},
        {keys::input::kCursorDecoration, {
            {keys::input::cursor_decoration::kEnabled, input.cursorDecoration.enabled},
            {keys::input::cursor_decoration::kPluginId, input.cursorDecoration.pluginId},
            {keys::input::cursor_decoration::kColorHex, input.cursorDecoration.colorHex},
            {keys::input::cursor_decoration::kSizePx, input.cursorDecoration.sizePx},
            {keys::input::cursor_decoration::kAlphaPercent, input.cursorDecoration.alphaPercent},
        }}
    };
}

} // namespace mousefx::config_json::serialize_internal
