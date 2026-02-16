#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::parse_internal {
namespace {

void ParseRippleEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains(keys::effect::kRipple) || !effects[keys::effect::kRipple].is_object()) {
        return;
    }

    const auto& ripple = effects[keys::effect::kRipple];
    config.ripple.durationMs = GetOr<int>(ripple, keys::effect::kDurationMs, config.ripple.durationMs);
    config.ripple.startRadius = GetOr<float>(ripple, keys::effect::kStartRadius, config.ripple.startRadius);
    config.ripple.endRadius = GetOr<float>(ripple, keys::effect::kEndRadius, config.ripple.endRadius);
    config.ripple.strokeWidth = GetOr<float>(ripple, keys::effect::kStrokeWidth, config.ripple.strokeWidth);
    config.ripple.windowSize = GetOr<int>(ripple, keys::effect::kWindowSize, config.ripple.windowSize);

    if (ripple.contains(keys::effect::click::kLeft) && ripple[keys::effect::click::kLeft].is_object()) {
        const auto& left = ripple[keys::effect::click::kLeft];
        config.ripple.leftClick.fill = GetColorOr(left, keys::effect::click::kFill, config.ripple.leftClick.fill);
        config.ripple.leftClick.stroke = GetColorOr(left, keys::effect::click::kStroke, config.ripple.leftClick.stroke);
        config.ripple.leftClick.glow = GetColorOr(left, keys::effect::click::kGlow, config.ripple.leftClick.glow);
    }
    if (ripple.contains(keys::effect::click::kRight) && ripple[keys::effect::click::kRight].is_object()) {
        const auto& right = ripple[keys::effect::click::kRight];
        config.ripple.rightClick.fill = GetColorOr(right, keys::effect::click::kFill, config.ripple.rightClick.fill);
        config.ripple.rightClick.stroke = GetColorOr(right, keys::effect::click::kStroke, config.ripple.rightClick.stroke);
        config.ripple.rightClick.glow = GetColorOr(right, keys::effect::click::kGlow, config.ripple.rightClick.glow);
    }
    if (ripple.contains(keys::effect::click::kMiddle) && ripple[keys::effect::click::kMiddle].is_object()) {
        const auto& middle = ripple[keys::effect::click::kMiddle];
        config.ripple.middleClick.fill = GetColorOr(middle, keys::effect::click::kFill, config.ripple.middleClick.fill);
        config.ripple.middleClick.stroke = GetColorOr(middle, keys::effect::click::kStroke, config.ripple.middleClick.stroke);
        config.ripple.middleClick.glow = GetColorOr(middle, keys::effect::click::kGlow, config.ripple.middleClick.glow);
    }
}

void ParseTrailEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains(keys::effect::kTrail) || !effects[keys::effect::kTrail].is_object()) {
        return;
    }

    const auto& trail = effects[keys::effect::kTrail];
    config.trail.durationMs = GetOr<int>(trail, keys::effect::kDurationMs, config.trail.durationMs);
    config.trail.maxPoints = GetOr<int>(trail, keys::profile::kMaxPoints, config.trail.maxPoints);
    config.trail.lineWidth = GetOr<float>(trail, keys::effect::kLineWidth, config.trail.lineWidth);
    config.trail.color = GetColorOr(trail, keys::effect::kColor, config.trail.color);
}

void ParseIconEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains(keys::effect::kIconStar) || !effects[keys::effect::kIconStar].is_object()) {
        return;
    }

    const auto& icon = effects[keys::effect::kIconStar];
    config.icon.durationMs = GetOr<int>(icon, keys::effect::kDurationMs, config.icon.durationMs);
    config.icon.startRadius = GetOr<float>(icon, keys::effect::kStartRadius, config.icon.startRadius);
    config.icon.endRadius = GetOr<float>(icon, keys::effect::kEndRadius, config.icon.endRadius);
    config.icon.strokeWidth = GetOr<float>(icon, keys::effect::kStrokeWidth, config.icon.strokeWidth);
    config.icon.fillColor = GetColorOr(icon, keys::effect::click::kFill, config.icon.fillColor);
    config.icon.strokeColor = GetColorOr(icon, keys::effect::click::kStroke, config.icon.strokeColor);
}

void ParseTextEffect(const nlohmann::json& effects, EffectConfig& config) {
    if (!effects.contains(keys::effect::kTextClick) || !effects[keys::effect::kTextClick].is_object()) {
        return;
    }

    const auto& text = effects[keys::effect::kTextClick];
    config.textClick.durationMs = GetOr<int>(text, keys::effect::kDurationMs, config.textClick.durationMs);
    config.textClick.floatDistance = GetOr<int>(text, keys::effect::kFloatDistance, config.textClick.floatDistance);
    config.textClick.fontSize = GetOr<float>(text, keys::effect::kFontSize, config.textClick.fontSize);

    if (text.contains(keys::effect::kFontFamily) && text[keys::effect::kFontFamily].is_string()) {
        std::wstring fontFamily;
        if (TryUtf8ToWide(text[keys::effect::kFontFamily].get<std::string>(), &fontFamily)) {
            config.textClick.fontFamily = fontFamily;
        }
    }

    if (text.contains(keys::effect::kTexts) && text[keys::effect::kTexts].is_array()) {
        config.textClick.texts.clear();
        for (const auto& item : text[keys::effect::kTexts]) {
            if (!item.is_string()) {
                continue;
            }

            std::wstring ws;
            if (TryUtf8ToWide(item.get<std::string>(), &ws)) {
                config.textClick.texts.push_back(ws);
            }
        }
    }

    if (text.contains(keys::effect::kColors) && text[keys::effect::kColors].is_array()) {
        config.textClick.colors.clear();
        for (const auto& item : text[keys::effect::kColors]) {
            if (item.is_string()) {
                config.textClick.colors.push_back(ArgbFromHex(item.get<std::string>()));
            }
        }
    }
}

} // namespace

void ParseEffects(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains(keys::kEffects) || !root[keys::kEffects].is_object()) {
        return;
    }

    const auto& effects = root[keys::kEffects];
    ParseRippleEffect(effects, config);
    ParseTrailEffect(effects, config);
    ParseIconEffect(effects, config);
    ParseTextEffect(effects, config);
}

} // namespace mousefx::config_json::parse_internal
