#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

#include <cstdint>
#include <string>
#include <vector>

namespace mousefx {

struct InputIndicatorDebugState {
    std::string lastAppliedLabel{};
    uint64_t applyCount{0};
};

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

    virtual bool ReadDebugState(InputIndicatorDebugState* outState) const {
        (void)outState;
        return false;
    }
    virtual bool RunMouseLabelProbe(std::vector<std::string>* outAppliedLabels) {
        (void)outAppliedLabels;
        return false;
    }
    virtual bool RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels) {
        (void)outAppliedLabels;
        return false;
    }
};

} // namespace mousefx
