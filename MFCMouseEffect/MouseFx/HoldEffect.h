#pragma once

#include "IMouseEffect.h"
#include "RippleWindowPool.h"

namespace mousefx {

// Hold effect: shows growing ring while button is held down.
class HoldEffect final : public IMouseEffect {
public:
    HoldEffect() = default;
    ~HoldEffect() override;

    EffectCategory Category() const override { return EffectCategory::Hold; }
    const char* TypeName() const override { return "charge"; }

    bool Initialize() override;
    void Shutdown() override;
    void OnHoldStart(const POINT& pt, int button) override;
    void OnHoldEnd() override;

private:
    RippleWindowPool pool_{};
    POINT holdPoint_{};
    int holdButton_ = 0;
};

} // namespace mousefx
