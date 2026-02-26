#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <cstdint>
#include <string>

namespace mousefx {

class MacosTrailPulseEffect final : public IMouseEffect {
public:
    MacosTrailPulseEffect(
        std::string effectType,
        std::string themeName,
        macos_effect_profile::TrailRenderProfile renderProfile,
        macos_effect_profile::TrailThrottleProfile throttleProfile);
    ~MacosTrailPulseEffect() override;

    EffectCategory Category() const override { return EffectCategory::Trail; }
    const char* TypeName() const override { return effectType_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnMouseMove(const ScreenPoint& pt) override;

private:
    static uint64_t CurrentTickMs();

    std::string effectType_{};
    std::string themeName_{};
    macos_effect_profile::TrailRenderProfile renderProfile_{};
    macos_effect_profile::TrailThrottleProfile throttleProfile_{};
    bool initialized_ = false;

    bool hasLastPoint_ = false;
    ScreenPoint lastPoint_{};
    uint64_t lastEmitTickMs_ = 0;
};

} // namespace mousefx
