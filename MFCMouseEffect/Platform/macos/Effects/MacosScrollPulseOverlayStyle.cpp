#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
namespace {

bool ContainsScrollToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

const macos_effect_profile::ScrollRenderProfile::DirectionColor& ResolveDirectionColor(
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    if (horizontal) {
        return (delta >= 0) ? profile.horizontalPositive : profile.horizontalNegative;
    }
    return (delta >= 0) ? profile.verticalPositive : profile.verticalNegative;
}

} // namespace

std::string NormalizeScrollType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty() || value == "none") {
        return "arrow";
    }
    if (ContainsScrollToken(value, "helix")) {
        return "helix";
    }
    if (ContainsScrollToken(value, "twinkle") || ContainsScrollToken(value, "stardust")) {
        return "twinkle";
    }
    if (ContainsScrollToken(value, "arrow") ||
        ContainsScrollToken(value, "direction") ||
        ContainsScrollToken(value, "indicator")) {
        return "arrow";
    }
    return "arrow";
}

NSColor* ScrollPulseStrokeColor(
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    return ArgbToNsColor(ResolveDirectionColor(horizontal, delta, profile).strokeArgb);
}

NSColor* ScrollPulseFillColor(
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    return ArgbToNsColor(ResolveDirectionColor(horizontal, delta, profile).fillArgb);
}

CGPathRef CreateScrollPulseDirectionArrowPath(CGRect bodyRect, bool horizontal, int delta) {
    const bool positive = (delta >= 0);
    const CGFloat size = 7.0;
    const CGFloat cx = horizontal
        ? (positive ? CGRectGetMaxX(bodyRect) - 9.0 : CGRectGetMinX(bodyRect) + 9.0)
        : CGRectGetMidX(bodyRect);
    const CGFloat cy = horizontal
        ? CGRectGetMidY(bodyRect)
        : (positive ? CGRectGetMaxY(bodyRect) - 9.0 : CGRectGetMinY(bodyRect) + 9.0);

    CGMutablePathRef path = CGPathCreateMutable();
    if (horizontal) {
        if (positive) {
            CGPathMoveToPoint(path, nullptr, cx + size, cy);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy + size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy - size * 0.8);
        } else {
            CGPathMoveToPoint(path, nullptr, cx - size, cy);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy + size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy - size * 0.8);
        }
    } else {
        if (positive) {
            CGPathMoveToPoint(path, nullptr, cx, cy + size);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy - size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy - size * 0.8);
        } else {
            CGPathMoveToPoint(path, nullptr, cx, cy - size);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy + size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy + size * 0.8);
        }
    }
    CGPathCloseSubpath(path);
    return path;
}
#endif

} // namespace mousefx::macos_scroll_pulse
