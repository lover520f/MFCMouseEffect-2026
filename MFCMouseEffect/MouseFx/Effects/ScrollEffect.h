#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/RippleWindowPool.h"
#include <string>

namespace mousefx {

// Scroll effect: shows directional arrow on mouse wheel.
class ScrollEffect final : public IMouseEffect {
public:
    explicit ScrollEffect(const std::string& themeName);
    ~ScrollEffect() override;

    EffectCategory Category() const override { return EffectCategory::Scroll; }
    const char* TypeName() const override { return "arrow"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnScroll(const ScrollEvent& event) override;

private:
    RippleWindowPool pool_{};
    RippleStyle style_{};
    bool isChromatic_ = false;
};

} // namespace mousefx
