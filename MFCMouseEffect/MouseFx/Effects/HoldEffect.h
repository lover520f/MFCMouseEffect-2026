#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Interfaces/IHoldRuntime.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <cstdint>
#include <memory>
#include <string>

namespace mousefx {

// Hold effect: shows growing ring while button is held down.
class HoldEffect final : public IMouseEffect {
public:
    explicit HoldEffect(
        const std::string& themeName,
        const std::string& type,
        const std::string& followMode,
        const std::string& presenterBackend);
    ~HoldEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hold; }
    const char* TypeName() const override { return type_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoldStart(const ScreenPoint& pt, int button) override;
    void OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) override;
    void OnHoldEnd() override;
    void OnCommand(const std::string& cmd, const std::string& args) override;

private:
    enum class FollowMode : uint8_t {
        Precise = 0,
        Smooth,
        Efficient,
    };

    static FollowMode ParseFollowMode(const std::string& mode);
    static bool IsSamePoint(const ScreenPoint& a, const ScreenPoint& b);

    ScreenPoint holdPoint_{};
    int holdButton_ = 0;

    std::string type_;
    std::unique_ptr<IHoldRuntime> runtime_;
    RippleStyle style_{};
    FollowMode followMode_ = FollowMode::Smooth;
    bool hasSmoothedPoint_ = false;
    float smoothedX_ = 0.0f;
    float smoothedY_ = 0.0f;
    uint64_t lastEfficientPosMs_ = 0;
};

} // namespace mousefx
