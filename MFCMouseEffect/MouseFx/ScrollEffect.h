#pragma once

#include "IMouseEffect.h"
#include "RippleWindowPool.h"

namespace mousefx {

// Scroll effect: shows directional arrow on mouse wheel.
class ScrollEffect final : public IMouseEffect {
public:
    ScrollEffect() = default;
    ~ScrollEffect() override;

    EffectCategory Category() const override { return EffectCategory::Scroll; }
    const char* TypeName() const override { return "arrow"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnScroll(const ScrollEvent& event) override;

private:
    RippleWindowPool pool_{};
};

} // namespace mousefx
