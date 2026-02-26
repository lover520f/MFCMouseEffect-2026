#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

namespace mousefx {

class MacosClickPulseEffect final : public IMouseEffect {
public:
    MacosClickPulseEffect(std::string effectType, std::string themeName, macos_effect_profile::ClickRenderProfile renderProfile);
    ~MacosClickPulseEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return effectType_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    std::string effectType_{};
    std::string themeName_{};
    macos_effect_profile::ClickRenderProfile renderProfile_{};
    bool initialized_ = false;
};

} // namespace mousefx
