#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include <string>

namespace mousefx {

// Click effect that draws a star icon instead of a circle.
class IconEffect final : public IMouseEffect {
public:
    IconEffect(const std::string& themeName, const EffectConfig& config);
    ~IconEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "star"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    ClickEffectProfile profile_{};
    bool isChromatic_ = false;
};

} // namespace mousefx
