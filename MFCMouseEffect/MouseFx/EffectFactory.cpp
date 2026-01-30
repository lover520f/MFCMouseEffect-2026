#include "pch.h"

#include "EffectFactory.h"

#include "HoldEffect.h"
#include "HoverEffect.h"
#include "IconEffect.h"
#include "ParticleTrailEffect.h"
#include "RippleEffect.h"
#include "ScrollEffect.h"
#include "TextEffect.h"
#include "TrailEffect.h"

namespace mousefx {

std::unique_ptr<IMouseEffect> EffectFactory::Create(EffectCategory category, const std::string& type, const EffectConfig& config) {
    if (type == "none" || type.empty()) {
        return nullptr;
    }

    switch (category) {
        case EffectCategory::Click:
            if (type == "ripple") return std::make_unique<RippleEffect>(config.theme);
            if (type == "star")   return std::make_unique<IconEffect>(config.theme);
            if (type == "text")   return std::make_unique<TextEffect>(config.textClick, config.theme);
            break;
        case EffectCategory::Trail:
            if (type == "particle") return std::make_unique<ParticleTrailEffect>(config.theme);
            // "line", "streamer", "electric" are handled by TrailEffect via strategy.
            return std::make_unique<TrailEffect>(config.theme, type);
        case EffectCategory::Scroll:
            if (type == "arrow") return std::make_unique<ScrollEffect>(config.theme);
            break;
        case EffectCategory::Hold:
            return std::make_unique<HoldEffect>(config.theme, type);
        case EffectCategory::Hover:
            return std::make_unique<HoverEffect>(config.theme, type);
        case EffectCategory::Edge:
            break;
        default:
            break;
    }

#ifdef _DEBUG
    OutputDebugStringA(("MouseFx: unknown effect type: " + type + "\n").c_str());
#endif
    return nullptr;
}

} // namespace mousefx

