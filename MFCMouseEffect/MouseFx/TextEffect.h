#pragma once

#include "IMouseEffect.h"
#include "TextWindowPool.h"
#include "EffectConfig.h"

namespace mousefx {

class TextEffect final : public IMouseEffect {
public:
    TextEffect(const TextConfig& config);
    ~TextEffect() override;

    EffectCategory Category() const override { return EffectCategory::Click; }
    const char* TypeName() const override { return "text"; }

    bool Initialize() override;
    void Shutdown() override;

    void OnClick(const ClickEvent& event) override;

private:
    TextConfig config_;
    TextWindowPool pool_{};
};

} // namespace mousefx
