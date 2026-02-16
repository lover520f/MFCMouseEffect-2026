#include "pch.h"
#include "EffectConfigJsonCodecEffectsColorHelpers.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::effects_color_helpers {
namespace {

Argb ParseHexColorOrDefault(const nlohmann::json& section, const char* key, Argb defaultValue) {
    if (section.contains(key) && section[key].is_string()) {
        return ArgbFromHex(section[key].get<std::string>());
    }
    return defaultValue;
}

} // namespace

void ParseRippleButtonColors(const nlohmann::json& section, RippleConfig::ButtonColors& colors) {
    colors.fill = ParseHexColorOrDefault(section, keys::effect::click::kFill, colors.fill);
    colors.stroke = ParseHexColorOrDefault(section, keys::effect::click::kStroke, colors.stroke);
    colors.glow = ParseHexColorOrDefault(section, keys::effect::click::kGlow, colors.glow);
}

void ParseFillStrokeColors(const nlohmann::json& section, Argb& fill, Argb& stroke) {
    fill = ParseHexColorOrDefault(section, keys::effect::click::kFill, fill);
    stroke = ParseHexColorOrDefault(section, keys::effect::click::kStroke, stroke);
}

nlohmann::json BuildRippleButtonColors(const RippleConfig::ButtonColors& colors) {
    return {
        {keys::effect::click::kFill, config_internal::ArgbToHex(colors.fill)},
        {keys::effect::click::kStroke, config_internal::ArgbToHex(colors.stroke)},
        {keys::effect::click::kGlow, config_internal::ArgbToHex(colors.glow)}
    };
}

nlohmann::json BuildFillStrokeColors(Argb fill, Argb stroke) {
    return {
        {keys::effect::click::kFill, config_internal::ArgbToHex(fill)},
        {keys::effect::click::kStroke, config_internal::ArgbToHex(stroke)}
    };
}

} // namespace mousefx::config_json::effects_color_helpers
