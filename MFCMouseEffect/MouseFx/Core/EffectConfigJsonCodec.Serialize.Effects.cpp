#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonCodecEffectsColorHelpers.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildActiveEffectsJson(const ActiveEffectConfig& active) {
    return {
        {keys::active::kClick, active.click},
        {keys::active::kTrail, active.trail},
        {keys::active::kScroll, active.scroll},
        {keys::active::kHover, active.hover},
        {keys::active::kHold, active.hold}
    };
}

nlohmann::json BuildEffectsJson(const EffectConfig& config) {
    nlohmann::json effects;

    effects[keys::effect::kRipple] = {
        {keys::effect::kDurationMs, config.ripple.durationMs},
        {keys::effect::kStartRadius, config.ripple.startRadius},
        {keys::effect::kEndRadius, config.ripple.endRadius},
        {keys::effect::kStrokeWidth, config.ripple.strokeWidth},
        {keys::effect::kWindowSize, config.ripple.windowSize},
        {keys::effect::click::kLeft, effects_color_helpers::BuildRippleButtonColors(config.ripple.leftClick)},
        {keys::effect::click::kRight, effects_color_helpers::BuildRippleButtonColors(config.ripple.rightClick)},
        {keys::effect::click::kMiddle, effects_color_helpers::BuildRippleButtonColors(config.ripple.middleClick)}
    };

    effects[keys::effect::kTrail] = {
        {keys::effect::kDurationMs, config.trail.durationMs},
        {keys::profile::kMaxPoints, config.trail.maxPoints},
        {keys::effect::kLineWidth, config.trail.lineWidth},
        {keys::effect::kColor, config_internal::ArgbToHex(config.trail.color)}
    };

    auto iconJson = nlohmann::json{
        {keys::effect::kDurationMs, config.icon.durationMs},
        {keys::effect::kStartRadius, config.icon.startRadius},
        {keys::effect::kEndRadius, config.icon.endRadius},
        {keys::effect::kStrokeWidth, config.icon.strokeWidth}
    };
    iconJson.update(effects_color_helpers::BuildFillStrokeColors(config.icon.fillColor, config.icon.strokeColor));
    effects[keys::effect::kIconStar] = std::move(iconJson);

    nlohmann::json textClick;
    textClick[keys::effect::kDurationMs] = config.textClick.durationMs;
    textClick[keys::effect::kFloatDistance] = config.textClick.floatDistance;
    textClick[keys::effect::kFontSize] = config.textClick.fontSize;
    textClick[keys::effect::kFontFamily] = config_internal::WStringToUtf8(config.textClick.fontFamily);

    nlohmann::json texts = nlohmann::json::array();
    for (const auto& text : config.textClick.texts) {
        texts.push_back(config_internal::WStringToUtf8(text));
    }
    textClick[keys::effect::kTexts] = texts;

    nlohmann::json colors = nlohmann::json::array();
    for (const auto& color : config.textClick.colors) {
        colors.push_back(config_internal::ArgbToHex(color));
    }
    textClick[keys::effect::kColors] = colors;
    effects[keys::effect::kTextClick] = textClick;

    return effects;
}

} // namespace mousefx::config_json::serialize_internal
