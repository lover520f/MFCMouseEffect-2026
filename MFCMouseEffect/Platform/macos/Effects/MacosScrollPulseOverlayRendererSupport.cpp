#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

#include <algorithm>
#include <cmath>

namespace mousefx::macos_scroll_pulse::support {

int ResolveStrengthLevel(int delta) {
    int strengthLevel = static_cast<int>(std::abs(delta) / 120);
    if (strengthLevel < 1) {
        strengthLevel = 1;
    }
    if (strengthLevel > 6) {
        strengthLevel = 6;
    }
    return strengthLevel;
}

CGRect BuildBodyRect(CGFloat size, bool horizontal, int strengthLevel, double intensity) {
    const CGFloat bodyThickness = 18.0;
    const CGFloat intensityScale = static_cast<CGFloat>(std::clamp(intensity, 0.0, 1.0));
    const CGFloat levelLength = 56.0 + static_cast<CGFloat>(strengthLevel) * 8.0;
    const CGFloat smoothLength = 54.0 + intensityScale * 50.0;
    const CGFloat bodyLength = std::max(levelLength, smoothLength);
    return horizontal
        ? CGRectMake((size - bodyLength) * 0.5, (size - bodyThickness) * 0.5, bodyLength, bodyThickness)
        : CGRectMake((size - bodyThickness) * 0.5, (size - bodyLength) * 0.5, bodyThickness, bodyLength);
}

CFTimeInterval BuildPulseDuration(
    const macos_effect_profile::ScrollRenderProfile& profile,
    int strengthLevel,
    CGFloat overlaySize) {
    const CFTimeInterval strengthDuration =
        profile.baseDurationSec + static_cast<CFTimeInterval>(strengthLevel) * profile.perStrengthStepSec;
    return macos_overlay_support::ScaleOverlayDurationBySize(
        strengthDuration,
        overlaySize,
        160.0,
        0.16,
        1.60);
}

int BuildCloseAfterMs(
    const macos_effect_profile::ScrollRenderProfile& profile,
    CFTimeInterval duration) {
    return static_cast<int>(duration * 1000.0) + profile.closePaddingMs;
}

#if defined(__APPLE__)
namespace {

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

} // namespace

CAShapeLayer* CreateBodyLayer(
    CGRect bounds,
    CGRect bodyRect,
    double baseOpacity,
    uint32_t fillArgb,
    uint32_t strokeArgb) {
    CAShapeLayer* body = [CAShapeLayer layer];
    body.frame = bounds;
    CGPathRef bodyPath = CGPathCreateWithRoundedRect(bodyRect, 9.0, 9.0, nullptr);
    body.path = bodyPath;
    CGPathRelease(bodyPath);
    body.fillColor = [ArgbToNsColor(fillArgb) CGColor];
    body.strokeColor = [ArgbToNsColor(strokeArgb) CGColor];
    body.lineWidth = 2.0;
    body.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(baseOpacity, 0.0, 0.0));
    return body;
}

CAShapeLayer* CreateArrowLayer(
    CGRect bounds,
    CGRect bodyRect,
    bool horizontal,
    int delta,
    double baseOpacity,
    uint32_t strokeArgb) {
    CAShapeLayer* arrow = [CAShapeLayer layer];
    arrow.frame = bounds;
    CGPathRef arrowPath = CreateScrollPulseDirectionArrowPath(bodyRect, horizontal, delta);
    arrow.path = arrowPath;
    CGPathRelease(arrowPath);
    arrow.fillColor = [ArgbToNsColor(strokeArgb) CGColor];
    arrow.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(baseOpacity, 0.02, 0.0));
    return arrow;
}
#endif

} // namespace mousefx::macos_scroll_pulse::support
