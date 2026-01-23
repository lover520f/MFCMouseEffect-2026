#pragma once

#include "IMouseEffect.h"
#include "RippleWindowPool.h"

namespace mousefx {

class RippleEffect final : public IMouseEffect {
public:
    RippleEffect() = default;
    ~RippleEffect() override;

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    RippleWindowPool pool_{};
};

} // namespace mousefx
