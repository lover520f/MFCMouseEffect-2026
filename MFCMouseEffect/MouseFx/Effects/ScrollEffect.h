#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/RippleWindowPool.h"
#include <string>

namespace mousefx {

// Scroll effect: shows directional arrow on mouse wheel.
class ScrollEffect final : public IMouseEffect {
public:
    explicit ScrollEffect(const std::string& themeName, const std::string& rendererName = "arrow");
    ~ScrollEffect() override;

    EffectCategory Category() const override { return EffectCategory::Scroll; }
    const char* TypeName() const override { return currentRendererName_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnScroll(const ScrollEvent& event) override;
    void OnCommand(const std::string& cmd, const std::string& args) override;

private:
    RippleWindowPool pool_{};
    RippleStyle style_{};
    bool isChromatic_ = false;
    std::string currentRendererName_ = "arrow";
};

} // namespace mousefx
