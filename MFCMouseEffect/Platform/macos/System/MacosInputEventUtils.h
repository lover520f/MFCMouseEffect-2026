#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#if defined(__APPLE__)
#import <ApplicationServices/ApplicationServices.h>
#endif

namespace mousefx::macos_input_event {

MouseButton MouseButtonFromEvent(CGEventRef event);
bool IsMouseMoveType(CGEventType type);
bool IsMouseDownType(CGEventType type);
bool IsMouseUpType(CGEventType type);
ScreenPoint ToScreenPoint(const CGPoint& pt);

} // namespace mousefx::macos_input_event
