#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/TextWindowPool.h"
#include "MouseFx/Core/EffectConfig.h"

namespace mousefx {

class TextEffect final : public IMouseEffect {
public:
    TextEffect(const TextConfig& config, const std::string& themeName);
    ~TextEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "text"; }

    bool Initialize() override;
    void Shutdown() override;

    void OnClick(const ClickEvent& event) override;

private:
    TextConfig config_;
    TextWindowPool pool_{};
    bool isChromatic_ = false;
};

} // namespace mousefx
