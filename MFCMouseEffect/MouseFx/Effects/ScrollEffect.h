#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Styles/RippleStyle.h"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace mousefx {

// Scroll effect: emits renderer-driven directional one-shot visuals on mouse wheel.
class ScrollEffect final : public IMouseEffect {
public:
    explicit ScrollEffect(const EffectConfig& config, const std::string& rendererName = "arrow");
    ~ScrollEffect() override;

    EffectCategory Category() const override { return EffectCategory::Scroll; }
    const char* TypeName() const override { return currentRendererName_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnScroll(const ScrollEvent& event) override;
    void OnCommand(const std::string& cmd, const std::string& args) override;

private:
    void PruneInactiveRipples(size_t maxActive);

    RippleStyle style_{};
    ScrollEffectProfile computeProfile_{};
    bool isChromatic_ = false;
    int sizeScalePercent_ = 100;
    std::string currentRendererName_ = "arrow";
    uint64_t lastEmitTickMs_ = 0;
    int pendingDelta_ = 0;
    std::deque<uint64_t> activeRippleIds_{};
};

} // namespace mousefx
