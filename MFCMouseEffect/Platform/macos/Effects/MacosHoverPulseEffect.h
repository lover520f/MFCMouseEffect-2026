#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

namespace mousefx {

class MacosHoverPulseEffect final : public IMouseEffect {
public:
    MacosHoverPulseEffect(std::string effectType, std::string themeName, macos_effect_profile::HoverRenderProfile renderProfile);
    ~MacosHoverPulseEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hover; }
    const char* TypeName() const override { return effectType_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoverStart(const ScreenPoint& pt) override;
    void OnHoverEnd() override;

private:
    std::string effectType_{};
    std::string themeName_{};
    macos_effect_profile::HoverRenderProfile renderProfile_{};
    bool initialized_ = false;
};

} // namespace mousefx
