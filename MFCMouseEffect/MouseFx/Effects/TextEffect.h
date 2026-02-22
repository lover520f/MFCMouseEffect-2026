#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Interfaces/ITextEffectFallback.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include <memory>

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
    std::unique_ptr<ITextEffectFallback> fallback_;
    bool isChromatic_ = false;
};

} // namespace mousefx
