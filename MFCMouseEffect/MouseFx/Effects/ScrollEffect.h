#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Styles/RippleStyle.h"
#include <cstdint>
#include <deque>
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
    static constexpr uint64_t kHelixEmitIntervalMs = 14;
    static constexpr size_t kHelixMaxActiveRipples = 8;

    bool IsHelixRenderer() const;
    void PruneInactiveRipples(size_t maxActive);

    RippleStyle style_{};
    bool isChromatic_ = false;
    std::string currentRendererName_ = "arrow";
    uint64_t lastEmitTickMs_ = 0;
    int pendingDelta_ = 0;
    std::deque<uint64_t> activeRippleIds_{};
};

} // namespace mousefx
