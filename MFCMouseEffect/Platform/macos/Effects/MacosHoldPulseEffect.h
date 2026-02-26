#pragma once

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

private:
    enum class FollowMode {
        Precise,
        Smooth,
        Efficient,
    };

    static FollowMode ParseFollowMode(const std::string& mode);
    static uint64_t CurrentTickMs();

    std::string effectType_{};
    std::string themeName_{};
    macos_effect_profile::HoldRenderProfile renderProfile_{};
    FollowMode followMode_ = FollowMode::Smooth;
    bool initialized_ = false;
    bool running_ = false;

    MouseButton holdButton_ = MouseButton::Left;
    bool hasSmoothedPoint_ = false;
    float smoothedX_ = 0.0f;
    float smoothedY_ = 0.0f;
    uint64_t lastEfficientTickMs_ = 0;
};

} // namespace mousefx
