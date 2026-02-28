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
namespace {

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

const macos_effect_profile::ClickButtonColorProfile& ResolveClickButtonColorProfile(
    MouseButton button,
    const macos_effect_profile::ClickRenderProfile& profile) {
    switch (button) {
    case MouseButton::Right:
        return profile.rightButton;
    case MouseButton::Middle:
        return profile.middleButton;
    case MouseButton::Left:
    default:
        return profile.leftButton;
    }
}

} // namespace

std::string NormalizeClickType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "star" || value == "text") {
        return value;
    }
    return "ripple";
}

NSColor* ClickPulseStrokeColor(
    MouseButton button,
    const macos_effect_profile::ClickRenderProfile& profile) {
    return ArgbToNsColor(ResolveClickButtonColorProfile(button, profile).strokeArgb);
}

NSColor* ClickPulseFillColor(
    MouseButton button,
    const macos_effect_profile::ClickRenderProfile& profile) {
    return ArgbToNsColor(ResolveClickButtonColorProfile(button, profile).fillArgb);
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
