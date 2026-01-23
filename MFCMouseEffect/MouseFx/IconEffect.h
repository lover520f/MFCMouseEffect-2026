#pragma once

#include "IMouseEffect.h"
#include "RippleWindowPool.h"

namespace mousefx {

// Similar to RippleEffect, but draws a Star/Icon instead of a circle.
// We can reuse RippleWindowPool logic if we make RippleWindow drawing configurable,
// or we can subclass/modify RippleWindow.
// For "lightweight", let's make RippleStyle configurable? 
// No, let's just make a separate simple implementation reusing the Pool idea but maybe 
// just patching RippleStyle for now?
// Actually, let's do it properly: IconEffect reuses the pool infrastructure but sets a "DrawType".

class IconEffect final : public IMouseEffect {
public:
    IconEffect() = default;
    ~IconEffect() override;

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;

private:
    RippleWindowPool pool_{};
};

} // namespace mousefx
