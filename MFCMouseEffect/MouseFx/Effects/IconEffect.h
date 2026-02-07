#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Styles/RippleStyle.h"
#include <string>

namespace mousefx {

// Click effect that draws a star icon instead of a circle.
class IconEffect final : public IMouseEffect {
public:
    explicit IconEffect(const std::string& themeName);
    ~IconEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "star"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    RippleStyle style_{};
    bool isChromatic_ = false;
};

} // namespace mousefx
