#pragma once

#include "IMouseEffect.h"
#include "RippleWindowPool.h"

namespace mousefx {

// Click effect that draws a star icon instead of a circle.
class IconEffect final : public IMouseEffect {
public:
    IconEffect() = default;
    ~IconEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "star"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    RippleWindowPool pool_{};
};

} // namespace mousefx
