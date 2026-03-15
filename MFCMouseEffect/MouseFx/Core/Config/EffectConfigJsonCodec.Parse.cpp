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
    if (root.contains(keys::kEffectsBlacklistApps) && root[keys::kEffectsBlacklistApps].is_array()) {
        std::vector<std::string> apps;
        for (const auto& item : root[keys::kEffectsBlacklistApps]) {
            if (item.is_string()) {
                apps.push_back(item.get<std::string>());
            }
        }
        config.effectsBlacklistApps = config_internal::SanitizeEffectsBlacklistApps(std::move(apps));
    }
    config.trailStyle = parse_internal::GetOr<std::string>(root, keys::kTrailStyle, config.trailStyle);

    if (root.contains(keys::kActiveEffects) && root[keys::kActiveEffects].is_object()) {
        const auto& active = root[keys::kActiveEffects];
        config.active.click = parse_internal::GetOr<std::string>(active, keys::active::kClick, config.active.click);
        config.active.trail = parse_internal::GetOr<std::string>(active, keys::active::kTrail, config.active.trail);
        config.active.scroll = parse_internal::GetOr<std::string>(active, keys::active::kScroll, config.active.scroll);
        config.active.hover = parse_internal::GetOr<std::string>(active, keys::active::kHover, config.active.hover);
        config.active.hold = parse_internal::GetOr<std::string>(active, keys::active::kHold, config.active.hold);
    }

    if (root.contains(keys::kEffectSizeScales) && root[keys::kEffectSizeScales].is_object()) {
        const auto& scales = root[keys::kEffectSizeScales];
        config.effectSizeScales.click =
            parse_internal::GetOr<int>(scales, keys::effect_size_scale::kClick, config.effectSizeScales.click);
        config.effectSizeScales.trail =
            parse_internal::GetOr<int>(scales, keys::effect_size_scale::kTrail, config.effectSizeScales.trail);
        config.effectSizeScales.scroll =
            parse_internal::GetOr<int>(scales, keys::effect_size_scale::kScroll, config.effectSizeScales.scroll);
        config.effectSizeScales.hold =
            parse_internal::GetOr<int>(scales, keys::effect_size_scale::kHold, config.effectSizeScales.hold);
        config.effectSizeScales.hover =
            parse_internal::GetOr<int>(scales, keys::effect_size_scale::kHover, config.effectSizeScales.hover);
    }
    config.effectSizeScales = config_internal::SanitizeEffectSizeScaleConfig(config.effectSizeScales);

    if (root.contains(keys::kEffectConflictPolicy) && root[keys::kEffectConflictPolicy].is_object()) {
        const auto& policy = root[keys::kEffectConflictPolicy];
        config.effectConflictPolicy.holdMovePolicy = parse_internal::GetOr<std::string>(
            policy,
            keys::effect_conflict_policy::kHoldMovePolicy,
            config.effectConflictPolicy.holdMovePolicy);
        config.effectConflictPolicy.holdMovePolicy = parse_internal::GetOr<std::string>(
            policy,
            keys::effect_conflict_policy::kHoldMoveLegacy,
            config.effectConflictPolicy.holdMovePolicy);
    }
    config.effectConflictPolicy = config_internal::SanitizeEffectConflictPolicyConfig(config.effectConflictPolicy);

    parse_internal::ParseInputIndicator(root, config);
    parse_internal::ParseAutomation(root, config);
    parse_internal::ParseWasm(root, config);
    parse_internal::ParseTrailParams(root, config);
    parse_internal::ParseTrailProfiles(root, config);
    parse_internal::ParseEffects(root, config);
}

} // namespace mousefx::config_json
