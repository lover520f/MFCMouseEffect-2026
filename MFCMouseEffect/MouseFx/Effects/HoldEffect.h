#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Styles/RippleStyle.h"
#include <cstdint>
#include <string>

namespace mousefx {

// Hold effect: shows growing ring while button is held down.
class HoldEffect final : public IMouseEffect {
public:
    explicit HoldEffect(
        const std::string& themeName,
        const std::string& type,
        const std::string& followMode,
        bool fluxGpuV2D2dExperimentalEnabled);
    ~HoldEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hold; }
    const char* TypeName() const override { return type_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoldStart(const POINT& pt, int button) override;
    void OnHoldUpdate(const POINT& pt, DWORD durationMs) override;
    void OnHoldEnd() override;
    void OnCommand(const std::string& cmd, const std::string& args) override;

private:
    enum class FollowMode : uint8_t {
        Precise = 0,
        Smooth,
        Efficient,
    };

    static FollowMode ParseFollowMode(const std::string& mode);
    static bool IsSamePoint(const POINT& a, const POINT& b);
    static bool IsGpuV2RouteType(const std::string& type);
    void SendHoldStateCommand(DWORD durationMs, const POINT& pt) const;

    POINT holdPoint_{};
    int holdButton_ = 0;
    
    uint64_t currentRippleId_ = 0;
    RippleStyle style_{};
    std::string type_; // Renderer type name
    bool isGpuV2Route_ = false;
    bool fluxGpuV2D2dExperimentalEnabled_ = false;
    bool isChromatic_ = false;
    FollowMode followMode_ = FollowMode::Smooth;
    bool hasSmoothedPoint_ = false;
    float smoothedX_ = 0.0f;
    float smoothedY_ = 0.0f;
    POINT lastSentPoint_{};
    bool hasLastSentPoint_ = false;
    uint64_t lastHoldCommandMs_ = 0;
    uint64_t lastEfficientPosMs_ = 0;
};

} // namespace mousefx
