#include "pch.h"

#include "MouseFx/Core/Control/AutomationDispatchFeature.h"

#include "MouseFx/Core/Control/AppController.h"

namespace mousefx {

void AutomationDispatchFeature::OnClick(AppController& controller, const ClickEvent& ev) const {
    controller.InputAutomation().OnClick(ev);
}

void AutomationDispatchFeature::OnScroll(AppController& controller, int16_t delta) const {
    controller.InputAutomation().OnScroll(static_cast<short>(delta));
}

void AutomationDispatchFeature::OnMouseMove(AppController& controller, const ScreenPoint& pt) const {
    controller.InputAutomation().OnMouseMove(pt);
}

void AutomationDispatchFeature::OnButtonDown(AppController& controller, const ScreenPoint& pt, int button) const {
    controller.InputAutomation().OnButtonDown(pt, button);
}

void AutomationDispatchFeature::OnButtonUp(AppController& controller, const ScreenPoint& pt, int button) const {
    controller.InputAutomation().OnButtonUp(pt, button);
}

void AutomationDispatchFeature::OnSuppressed(AppController& controller) const {
    controller.InputAutomation().Reset();
}

} // namespace mousefx
