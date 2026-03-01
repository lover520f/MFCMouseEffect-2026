#pragma once

#include <string>

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
std::string NormalizeScrollType(const std::string& effectType);
CGPathRef CreateScrollPulseDirectionArrowPath(CGRect bodyRect, bool horizontal, int delta);
#endif

} // namespace mousefx::macos_scroll_pulse
