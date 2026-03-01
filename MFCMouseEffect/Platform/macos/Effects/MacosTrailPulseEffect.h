#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosTrailPulseEmissionPlanner.h"

#include <cstdint>
#include <string>

namespace mousefx {

struct TrailPulseRuntimeDiagnostics final {
    uint64_t moveSamples = 0;
    uint64_t originConnectorDropCount = 0;
    uint64_t teleportDropCount = 0;
};

TrailPulseRuntimeDiagnostics ReadTrailPulseRuntimeDiagnostics();

class MacosTrailPulseEffect final : public IMouseEffect {
public:
    MacosTrailPulseEffect(
        std::string effectType,
        std::string themeName,
        macos_effect_profile::TrailRenderProfile renderProfile,
        macos_effect_profile::TrailThrottleProfile throttleProfile,
        IdleFadeParams idleFade,
        float lineWidth);
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
    IdleFadeParams idleFade_{};
    float lineWidth_ = 4.0f;
    bool initialized_ = false;

    bool hasLastPoint_ = false;
    ScreenPoint lastPoint_{};
    uint64_t lastEmitTickMs_ = 0;
    macos_trail_pulse::TrailPulseEmissionPlannerConfig emissionPlannerConfig_{};
    bool continuousTrailActive_ = false;
};

} // namespace mousefx
