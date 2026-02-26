#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
std::string NormalizeClickType(const std::string& effectType);
NSColor* ClickPulseStrokeColor(MouseButton button);
NSColor* ClickPulseFillColor(MouseButton button);
CGPathRef CreateClickPulseStarPath(CGRect bounds, int points);
#endif

} // namespace mousefx::macos_click_pulse
