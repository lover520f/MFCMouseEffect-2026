#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Interfaces/IHoldRuntime.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <cstdint>
#include <memory>
#include <string>

namespace mousefx {

// Hold effect: shows growing ring while button is held down.
class HoldEffect final : public IMouseEffect {
public:
    explicit HoldEffect(
        const std::string& themeName,
        const std::string& type,
        const std::string& followMode,
        const std::string& presenterBackend);
    ~HoldEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hold; }
    const char* TypeName() const override { return type_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoldStart(const ScreenPoint& pt, int button) override;
    void OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) override;
    void OnHoldEnd() override;
    void OnCommand(const std::string& cmd, const std::string& args) override;

private:
    ScreenPoint holdPoint_{};
    int holdButton_ = 0;

    std::string type_;
    std::unique_ptr<IHoldRuntime> runtime_;
    RippleStyle style_{};
    HoldEffectProfile computeProfile_{};
    HoldEffectFollowMode followMode_ = HoldEffectFollowMode::Smooth;
    HoldEffectFollowState followState_{};
};

} // namespace mousefx
