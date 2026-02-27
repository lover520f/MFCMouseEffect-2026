#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <cstdint>
#include <string>

namespace mousefx {

class MacosScrollPulseEffect final : public IMouseEffect {
public:
    MacosScrollPulseEffect(std::string effectType, std::string themeName, macos_effect_profile::ScrollRenderProfile renderProfile);
    ~MacosScrollPulseEffect() override;

    EffectCategory Category() const override { return EffectCategory::Scroll; }
    const char* TypeName() const override { return effectType_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnScroll(const ScrollEvent& event) override;

private:
    static uint64_t CurrentTickMs();
    std::string effectType_{};
    std::string themeName_{};
    macos_effect_profile::ScrollRenderProfile renderProfile_{};
    bool initialized_ = false;
    uint64_t lastEmitTickMs_ = 0;
    int pendingDelta_ = 0;
};

} // namespace mousefx
