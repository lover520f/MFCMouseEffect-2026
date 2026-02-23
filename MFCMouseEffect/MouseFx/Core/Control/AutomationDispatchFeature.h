#pragma once

#include <cstdint>

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class AppController;

// Dispatch adapter for input-automation actions.
class AutomationDispatchFeature final {
public:
    void OnClick(AppController& controller, const ClickEvent& ev) const;
    void OnScroll(AppController& controller, int16_t delta) const;
    void OnMouseMove(AppController& controller, const ScreenPoint& pt) const;
    void OnButtonDown(AppController& controller, const ScreenPoint& pt, int button) const;
    void OnButtonUp(AppController& controller, const ScreenPoint& pt, int button) const;
    void OnSuppressed(AppController& controller) const;
};

} // namespace mousefx
