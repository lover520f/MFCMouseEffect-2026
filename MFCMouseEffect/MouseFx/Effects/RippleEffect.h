#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/RippleWindowPool.h"
#include <string>

namespace mousefx {

class RippleEffect final : public IMouseEffect {
public:
    explicit RippleEffect(const std::string& themeName);
    ~RippleEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "ripple"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    RippleWindowPool pool_{};
    RippleStyle style_{};
    bool isChromatic_ = false;
};

} // namespace mousefx
