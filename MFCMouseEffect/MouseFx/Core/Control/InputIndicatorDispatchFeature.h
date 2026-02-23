#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class AppController;
struct ScrollEvent;

// Dispatch adapter for input-indicator overlay notifications.
class InputIndicatorDispatchFeature final {
public:
    void OnClick(AppController& controller, const ClickEvent& ev) const;
    void OnScroll(AppController& controller, const ScrollEvent& ev) const;
};

} // namespace mousefx
