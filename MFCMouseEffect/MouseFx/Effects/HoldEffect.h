#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/RippleWindowPool.h"
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
    
    void OnCommand(const std::string& cmd, const std::string& args) override {
        pool_.BroadcastCommand(cmd, args);
    }

private:
    RippleWindowPool pool_{};
    POINT holdPoint_{};
    int holdButton_ = 0;
    
    // Track active window to stop looping
    RippleWindow* currentRipple_ = nullptr;
    RippleStyle style_{};
    std::string type_; // Renderer type name
    bool isChromatic_ = false;
};

} // namespace mousefx
