#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

namespace mousefx {

// Platform-agnostic input indicator overlay contract.
class IInputIndicatorOverlay {
public:
    virtual ~IInputIndicatorOverlay() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Hide() = 0;
    virtual void UpdateConfig(const InputIndicatorConfig& cfg) = 0;

    virtual void OnClick(const ClickEvent& ev) = 0;
    virtual void OnScroll(const ScrollEvent& ev) = 0;
    virtual void OnKey(const KeyEvent& ev) = 0;
};

} // namespace mousefx
