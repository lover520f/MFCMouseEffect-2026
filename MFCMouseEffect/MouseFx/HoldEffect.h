#pragma once

#include "IMouseEffect.h"
#include "RippleWindowPool.h"
#include <string>

namespace mousefx {

// Hold effect: shows growing ring while button is held down.
class HoldEffect final : public IMouseEffect {
public:
    enum class Mode { Charge, Lightning };
    explicit HoldEffect(const std::string& themeName, Mode mode);
    ~HoldEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hold; }
    const char* TypeName() const override { return mode_ == Mode::Lightning ? "lightning" : "charge"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoldStart(const POINT& pt, int button) override;
    void OnHoldUpdate(const POINT& pt, DWORD durationMs) override;
    void OnHoldEnd() override;

private:
    RippleWindowPool pool_{};
    POINT holdPoint_{};
    int holdButton_ = 0;
    
    // Track active window to stop looping
    RippleWindow* currentRipple_ = nullptr;
    RippleStyle style_{};
    Mode mode_ = Mode::Charge;
};

} // namespace mousefx
