#pragma once

#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"

namespace mousefx {

class NullInputIndicatorOverlay final : public IInputIndicatorOverlay {
public:
    bool Initialize() override { return true; }
    void Shutdown() override {}
    void Hide() override {}
    void UpdateConfig(const InputIndicatorConfig& /*cfg*/) override {}
    void OnClick(const ClickEvent& /*ev*/) override {}
    void OnScroll(const ScrollEvent& /*ev*/) override {}
    void OnKey(const KeyEvent& /*ev*/) override {}
};

} // namespace mousefx
