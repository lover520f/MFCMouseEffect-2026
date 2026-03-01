#pragma once

#include <string>

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
std::string NormalizeClickType(const std::string& effectType);
CGPathRef CreateClickPulseStarPath(CGRect bounds, int points);
#endif

} // namespace mousefx::macos_click_pulse
