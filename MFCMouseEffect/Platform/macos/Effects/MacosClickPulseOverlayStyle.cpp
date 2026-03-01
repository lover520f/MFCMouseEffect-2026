#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#include <algorithm>
#include <cmath>
#endif

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
namespace {

} // namespace

std::string NormalizeClickType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "star" || value == "text") {
        return value;
    }
    return "ripple";
}

CGPathRef CreateClickPulseStarPath(CGRect bounds, int points) {
    const int safePoints = std::max(4, points);
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    const CGFloat outerRadius = std::min(CGRectGetWidth(bounds), CGRectGetHeight(bounds)) * 0.42;
    const CGFloat innerRadius = outerRadius * 0.46;
    const CGFloat startAngle = -M_PI_2;

    CGMutablePathRef path = CGPathCreateMutable();
    for (int i = 0; i < safePoints * 2; ++i) {
        const CGFloat radius = (i % 2 == 0) ? outerRadius : innerRadius;
        const CGFloat angle = startAngle + static_cast<CGFloat>(i) * static_cast<CGFloat>(M_PI) / safePoints;
        const CGFloat x = cx + std::cos(angle) * radius;
        const CGFloat y = cy + std::sin(angle) * radius;
        if (i == 0) {
            CGPathMoveToPoint(path, nullptr, x, y);
        } else {
            CGPathAddLineToPoint(path, nullptr, x, y);
        }
    }
    CGPathCloseSubpath(path);
    return path;
}
#endif

} // namespace mousefx::macos_click_pulse
