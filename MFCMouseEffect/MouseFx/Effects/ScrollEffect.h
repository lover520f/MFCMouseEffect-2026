#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Styles/RippleStyle.h"
#include <cstddef>
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
    struct InputShaperProfile {
        uint64_t emitIntervalMs = 10;
        size_t maxActiveRipples = 12;
        uint32_t maxDurationMs = 320;
    };

    static constexpr uint64_t kHelixEmitIntervalMs = 14;
    static constexpr size_t kHelixMaxActiveRipples = 8;
    static constexpr uint32_t kHelixMaxDurationMs = 240;

    bool IsHelixRenderer() const;
    InputShaperProfile GetInputShaperProfile() const;
    void PruneInactiveRipples(size_t maxActive);

    RippleStyle style_{};
    bool isChromatic_ = false;
    std::string currentRendererName_ = "arrow";
    uint64_t lastEmitTickMs_ = 0;
    int pendingDelta_ = 0;
    std::deque<uint64_t> activeRippleIds_{};
};

} // namespace mousefx
