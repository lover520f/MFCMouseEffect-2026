#include "pch.h"

#include "Platform/macos/Effects/MacosEffectCreatorRegistry.Internal.h"

#include "Platform/macos/Effects/MacosClickPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosHoldPulseEffect.h"
#include "Platform/macos/Effects/MacosHoverPulseEffect.h"
#include "Platform/macos/Effects/MacosScrollPulseEffect.h"
#include "Platform/macos/Effects/MacosTrailPulseEffect.h"
#include "MouseFx/Effects/TextEffect.h"

namespace mousefx::macos_effect_registry::detail {
namespace {

std::unique_ptr<IMouseEffect> CreateClick(const std::string& type, const EffectConfig& config) {
    return std::make_unique<MacosClickPulseEffect>(
        type,
        config.theme,
        macos_effect_profile::ResolveClickRenderProfile(config),
        config.textClick);
}

std::unique_ptr<IMouseEffect> CreateClickText(const std::string&, const EffectConfig& config) {
    return std::make_unique<TextEffect>(config.textClick, config.theme);
}

std::unique_ptr<IMouseEffect> CreateTrail(const std::string& type, const EffectConfig& config) {
    return std::make_unique<MacosTrailPulseEffect>(
        type,
        config.theme,
        macos_effect_profile::ResolveTrailRenderProfile(config, type),
        macos_effect_profile::ResolveTrailThrottleProfile(config, type),
        config.trailParams.idleFade,
        config.trail.lineWidth);
}

std::unique_ptr<IMouseEffect> CreateScroll(const std::string& type, const EffectConfig& config) {
    return std::make_unique<MacosScrollPulseEffect>(
        type,
        config.theme,
        macos_effect_profile::ResolveScrollRenderProfile(config));
}

std::unique_ptr<IMouseEffect> CreateHold(const std::string& type, const EffectConfig& config) {
    return std::make_unique<MacosHoldPulseEffect>(
        type,
        config.theme,
        config.holdFollowMode,
        macos_effect_profile::ResolveHoldRenderProfile(config));
}

std::unique_ptr<IMouseEffect> CreateHover(const std::string& type, const EffectConfig& config) {
    return std::make_unique<MacosHoverPulseEffect>(
        type,
        config.theme,
        macos_effect_profile::ResolveHoverRenderProfile(config));
}

} // namespace

const std::array<CategoryRegistryEntry, CategoryIndex(EffectCategory::Count)>& RegistryTable() {
    static const std::array<CategoryRegistryEntry, CategoryIndex(EffectCategory::Count)> table = [] {
        std::array<CategoryRegistryEntry, CategoryIndex(EffectCategory::Count)> result{};

        auto& click = result[CategoryIndex(EffectCategory::Click)];
        click.fallbackCreator = &CreateClick;
        click.typedCreators.emplace("ripple", &CreateClick);
        click.typedCreators.emplace("star", &CreateClick);
        click.typedCreators.emplace("text", &CreateClickText);

        auto& trail = result[CategoryIndex(EffectCategory::Trail)];
        trail.fallbackCreator = &CreateTrail;

        auto& scroll = result[CategoryIndex(EffectCategory::Scroll)];
        scroll.fallbackCreator = &CreateScroll;
        scroll.typedCreators.emplace("arrow", &CreateScroll);
        scroll.typedCreators.emplace("helix", &CreateScroll);
        scroll.typedCreators.emplace("twinkle", &CreateScroll);

        auto& hold = result[CategoryIndex(EffectCategory::Hold)];
        hold.fallbackCreator = &CreateHold;

        auto& hover = result[CategoryIndex(EffectCategory::Hover)];
        hover.fallbackCreator = &CreateHover;

        return result;
    }();
    return table;
}

} // namespace mousefx::macos_effect_registry::detail
