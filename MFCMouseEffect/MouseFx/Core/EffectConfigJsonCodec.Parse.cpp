#include "pch.h"
#include "EffectConfigJsonCodec.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonCodecParseInternal.h"

namespace mousefx::config_json {

void ApplyRootToConfig(const nlohmann::json& root, EffectConfig& config) {
    config.defaultEffect = parse_internal::GetOr<std::string>(root, "default_effect", config.defaultEffect);
    config.theme = parse_internal::GetOr<std::string>(root, "theme", config.theme);
    config.uiLanguage = parse_internal::GetOr<std::string>(root, "ui_language", config.uiLanguage);
    config.holdFollowMode = config_internal::NormalizeHoldFollowMode(
        parse_internal::GetOr<std::string>(root, "hold_follow_mode", config.holdFollowMode));
    config.trailStyle = parse_internal::GetOr<std::string>(root, "trail_style", config.trailStyle);

    if (root.contains("active_effects") && root["active_effects"].is_object()) {
        const auto& active = root["active_effects"];
        config.active.click = parse_internal::GetOr<std::string>(active, "click", config.active.click);
        config.active.trail = parse_internal::GetOr<std::string>(active, "trail", config.active.trail);
        config.active.scroll = parse_internal::GetOr<std::string>(active, "scroll", config.active.scroll);
        config.active.hover = parse_internal::GetOr<std::string>(active, "hover", config.active.hover);
        config.active.hold = parse_internal::GetOr<std::string>(active, "hold", config.active.hold);
    }

    parse_internal::ParseInputIndicator(root, config);
    parse_internal::ParseTrailParams(root, config);
    parse_internal::ParseTrailProfiles(root, config);
    parse_internal::ParseEffects(root, config);
}

} // namespace mousefx::config_json
