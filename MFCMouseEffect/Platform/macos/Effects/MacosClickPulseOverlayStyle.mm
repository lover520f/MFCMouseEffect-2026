#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#include <algorithm>
#include <cmath>
#endif

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
std::string NormalizeClickType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "star" || value == "text") {
        return value;
    }
    return "ripple";
}

NSColor* ClickPulseStrokeColor(MouseButton button) {
    switch (button) {
    case MouseButton::Left:
        return [NSColor colorWithCalibratedRed:0.22 green:0.70 blue:1 alpha:0.95];
    case MouseButton::Right:
        return [NSColor colorWithCalibratedRed:1.0 green:0.63 blue:0.22 alpha:0.95];
    case MouseButton::Middle:
        return [NSColor colorWithCalibratedRed:0.44 green:0.90 blue:0.57 alpha:0.95];
    default:
        return [NSColor colorWithCalibratedWhite:0.95 alpha:0.9];
    }
}

NSColor* ClickPulseFillColor(MouseButton button) {
    switch (button) {
    case MouseButton::Left:
        return [NSColor colorWithCalibratedRed:0.22 green:0.70 blue:1 alpha:0.22];
    case MouseButton::Right:
        return [NSColor colorWithCalibratedRed:1.0 green:0.63 blue:0.22 alpha:0.22];
    case MouseButton::Middle:
        return [NSColor colorWithCalibratedRed:0.44 green:0.90 blue:0.57 alpha:0.22];
    default:
        return [NSColor colorWithCalibratedWhite:0.95 alpha:0.18];
    }
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
