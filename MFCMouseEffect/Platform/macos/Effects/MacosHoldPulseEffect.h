#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <cstdint>
#include <string>

namespace mousefx {

class MacosHoldPulseEffect final : public IMouseEffect {
public:
    MacosHoldPulseEffect(
        std::string effectType,
        std::string themeName,
        std::string followMode,
        macos_effect_profile::HoldRenderProfile renderProfile);
    ~MacosHoldPulseEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hold; }
    const char* TypeName() const override { return effectType_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoldStart(const ScreenPoint& pt, int button) override;
    void OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) override;
    void OnHoldEnd() override;
    bool IsEffectActive() const override;

private:
    static uint64_t CurrentTickMs();

    std::string effectType_{};
    std::string themeName_{};
    macos_effect_profile::HoldRenderProfile renderProfile_{};
    HoldEffectFollowMode followMode_ = HoldEffectFollowMode::Smooth;
    bool initialized_ = false;
    bool running_ = false;

    MouseButton holdButton_ = MouseButton::Left;
    HoldEffectFollowState followState_{};
};

} // namespace mousefx
