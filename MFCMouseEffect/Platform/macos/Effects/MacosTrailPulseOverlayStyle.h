#pragma once

#include <string>

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace mousefx::macos_trail_pulse::detail {

#if defined(__APPLE__)
std::string NormalizeTrailType(const std::string& effectType);
CGPathRef CreateTrailLinePath(CGRect bounds, double deltaX, double deltaY, const std::string& trailType);
#endif

} // namespace mousefx::macos_trail_pulse::detail
