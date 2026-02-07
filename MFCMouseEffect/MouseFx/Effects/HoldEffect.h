#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Styles/RippleStyle.h"
#include <cstdint>
#include <string>

namespace mousefx {

// Hold effect: shows growing ring while button is held down.
class HoldEffect final : public IMouseEffect {
public:
    explicit HoldEffect(const std::string& themeName, const std::string& type);
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
    POINT holdPoint_{};
    int holdButton_ = 0;
    
    uint64_t currentRippleId_ = 0;
    RippleStyle style_{};
    std::string type_; // Renderer type name
    bool isChromatic_ = false;
};

} // namespace mousefx
