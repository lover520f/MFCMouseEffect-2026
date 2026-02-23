#include "pch.h"

#include "MouseFx/Core/Control/InputIndicatorDispatchFeature.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

namespace mousefx {

void InputIndicatorDispatchFeature::OnClick(AppController& controller, const ClickEvent& ev) const {
    controller.IndicatorOverlay().OnClick(ev);
}

void InputIndicatorDispatchFeature::OnScroll(AppController& controller, const ScrollEvent& ev) const {
    controller.IndicatorOverlay().OnScroll(ev);
}

} // namespace mousefx
