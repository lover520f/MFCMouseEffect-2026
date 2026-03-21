#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include <string>

namespace mousefx {

class RippleEffect final : public IMouseEffect {
public:
    RippleEffect(const std::string& themeName, const EffectConfig& config);
    ~RippleEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "ripple"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    ClickEffectProfile profile_{};
    bool isChromatic_ = false;
};

} // namespace mousefx
