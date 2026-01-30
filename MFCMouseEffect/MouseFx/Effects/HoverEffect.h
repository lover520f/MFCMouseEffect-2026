#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/RippleWindowPool.h"
#include <string>

namespace mousefx {

// Hover effect: shows a pulsing glow when the mouse is idle.
class HoverEffect final : public IMouseEffect {
public:
    explicit HoverEffect(const std::string& themeName, const std::string& type = "glow");
    ~HoverEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hover; }
    const char* TypeName() const override { return type_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    
    void OnHoverStart(const POINT& pt) override;
    void OnHoverEnd() override;

private:
    RippleWindowPool pool_{};
    RippleWindow* currentGlow_ = nullptr;
    RippleStyle style_{};
    std::string type_;
    bool isChromatic_ = false;
};

} // namespace mousefx
