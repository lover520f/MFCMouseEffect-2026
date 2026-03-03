#include "pch.h"
#include "EffectConfigJsonCodec.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"
#include "EffectConfigJsonCodecParseInternal.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::config_json {

void ApplyRootToConfig(const nlohmann::json& root, EffectConfig& config) {
    config.defaultEffect = parse_internal::GetOr<std::string>(root, keys::kDefaultEffect, config.defaultEffect);
    config.theme = parse_internal::GetOr<std::string>(root, keys::kTheme, config.theme);
    config.themeCatalogRootPath = TrimAscii(
        parse_internal::GetOr<std::string>(root, keys::kThemeCatalogRootPath, config.themeCatalogRootPath));
    config.overlayTargetFps = config_internal::SanitizeOverlayTargetFps(
        parse_internal::GetOr<int>(root, keys::kOverlayTargetFps, config.overlayTargetFps));
    config.uiLanguage = parse_internal::GetOr<std::string>(root, keys::kUiLanguage, config.uiLanguage);
    config.holdFollowMode = config_internal::NormalizeHoldFollowMode(
        parse_internal::GetOr<std::string>(root, keys::kHoldFollowMode, config.holdFollowMode));
    config.holdPresenterBackend = config_internal::NormalizeHoldPresenterBackend(
        parse_internal::GetOr<std::string>(root, keys::kHoldPresenterBackend, config.holdPresenterBackend));
    config.trailStyle = parse_internal::GetOr<std::string>(root, keys::kTrailStyle, config.trailStyle);

    if (root.contains(keys::kActiveEffects) && root[keys::kActiveEffects].is_object()) {
        const auto& active = root[keys::kActiveEffects];
        config.active.click = parse_internal::GetOr<std::string>(active, keys::active::kClick, config.active.click);
        config.active.trail = parse_internal::GetOr<std::string>(active, keys::active::kTrail, config.active.trail);
        config.active.scroll = parse_internal::GetOr<std::string>(active, keys::active::kScroll, config.active.scroll);
        config.active.hover = parse_internal::GetOr<std::string>(active, keys::active::kHover, config.active.hover);
        config.active.hold = parse_internal::GetOr<std::string>(active, keys::active::kHold, config.active.hold);
    }

    parse_internal::ParseInputIndicator(root, config);
    parse_internal::ParseAutomation(root, config);
    parse_internal::ParseWasm(root, config);
    parse_internal::ParseTrailParams(root, config);
    parse_internal::ParseTrailProfiles(root, config);
    parse_internal::ParseEffects(root, config);
}

} // namespace mousefx::config_json
