#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
NSColor* ScrollPulseStrokeColor(bool horizontal, int delta) {
    if (horizontal) {
        return (delta >= 0)
            ? [NSColor colorWithCalibratedRed:0.35 green:0.88 blue:0.95 alpha:0.96]
            : [NSColor colorWithCalibratedRed:0.62 green:0.80 blue:1 alpha:0.96];
    }
    return (delta >= 0)
        ? [NSColor colorWithCalibratedRed:0.42 green:0.92 blue:0.56 alpha:0.96]
        : [NSColor colorWithCalibratedRed:1.0 green:0.57 blue:0.34 alpha:0.96];
}

NSColor* ScrollPulseFillColor(bool horizontal, int delta) {
    if (horizontal) {
        return (delta >= 0)
            ? [NSColor colorWithCalibratedRed:0.35 green:0.88 blue:0.95 alpha:0.24]
            : [NSColor colorWithCalibratedRed:0.62 green:0.80 blue:1 alpha:0.24];
    }
    return (delta >= 0)
        ? [NSColor colorWithCalibratedRed:0.42 green:0.92 blue:0.56 alpha:0.24]
        : [NSColor colorWithCalibratedRed:1.0 green:0.57 blue:0.34 alpha:0.24];
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
