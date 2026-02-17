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
        {keys::input::kPerMonitorOverrides, perMonitorOverrides}
    };
}

} // namespace mousefx::config_json::serialize_internal
