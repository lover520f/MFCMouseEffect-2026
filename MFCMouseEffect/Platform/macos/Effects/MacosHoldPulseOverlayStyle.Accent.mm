#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.Internal.h"

#include <algorithm>
#include <cmath>

namespace mousefx::macos_hold_pulse::detail {
namespace {

CGPathRef CreateHexPath(CGRect bounds) {
    CGMutablePathRef path = CGPathCreateMutable();
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    const CGFloat radius = std::min(CGRectGetWidth(bounds), CGRectGetHeight(bounds)) * 0.42;
    for (int i = 0; i < 6; ++i) {
        const CGFloat angle = static_cast<CGFloat>(M_PI) / 3.0 * i - static_cast<CGFloat>(M_PI) / 2.0;
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

CGPathRef CreateLightningPath(CGRect bounds) {
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    const CGFloat h = CGRectGetHeight(bounds) * 0.40;
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, nullptr, cx - 6.0, cy + h * 0.45);
    CGPathAddLineToPoint(path, nullptr, cx + 2.0, cy + h * 0.10);
    CGPathAddLineToPoint(path, nullptr, cx - 1.5, cy + h * 0.10);
    CGPathAddLineToPoint(path, nullptr, cx + 6.0, cy - h * 0.45);
    CGPathAddLineToPoint(path, nullptr, cx - 2.0, cy - h * 0.05);
    CGPathAddLineToPoint(path, nullptr, cx + 1.5, cy - h * 0.05);
    CGPathCloseSubpath(path);
    return path;
}

CGPathRef CreateFluxFieldPath(CGRect bounds) {
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    const CGFloat r = std::min(CGRectGetWidth(bounds), CGRectGetHeight(bounds)) * 0.36;
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, nullptr, cx - r, cy);
    CGPathAddLineToPoint(path, nullptr, cx + r, cy);
    CGPathMoveToPoint(path, nullptr, cx, cy - r);
    CGPathAddLineToPoint(path, nullptr, cx, cy + r);
    return path;
}

bool ConfigureHexAccent(CAShapeLayer* accent, CGRect bounds, NSColor* baseColor) {
    CGPathRef path = CreateHexPath(CGRectInset(bounds, 38.0, 38.0));
    accent.path = path;
    CGPathRelease(path);
    accent.fillColor = [NSColor clearColor].CGColor;
    accent.strokeColor = [baseColor CGColor];
    accent.lineWidth = 1.8;
    return true;
}

bool ConfigureLightningAccent(CAShapeLayer* accent, CGRect bounds, NSColor* baseColor) {
    CGPathRef path = CreateLightningPath(CGRectInset(bounds, 36.0, 36.0));
    accent.path = path;
    CGPathRelease(path);
    accent.fillColor = [baseColor CGColor];
    accent.strokeColor = [baseColor CGColor];
    accent.lineWidth = 1.0;
    return true;
}

bool ConfigureFluxFieldAccent(CAShapeLayer* accent, CGRect bounds, NSColor* baseColor) {
    CGPathRef path = CreateFluxFieldPath(CGRectInset(bounds, 36.0, 36.0));
    accent.path = path;
    CGPathRelease(path);
    accent.fillColor = [NSColor clearColor].CGColor;
    accent.strokeColor = [baseColor CGColor];
    accent.lineWidth = 2.0;
    return true;
}

bool ConfigureQuantumHaloAccent(CAShapeLayer* accent, CGRect bounds, NSColor* baseColor) {
    CGPathRef path = CGPathCreateWithEllipseInRect(CGRectInset(bounds, 36.0, 36.0), nullptr);
    accent.path = path;
    CGPathRelease(path);
    accent.fillColor = [NSColor clearColor].CGColor;
    accent.strokeColor = [baseColor CGColor];
    accent.lineWidth = 2.2;
    return true;
}

} // namespace

bool ConfigureSpecialHoldAccentLayer(CAShapeLayer* accent, CGRect bounds, HoldStyle holdStyle, NSColor* baseColor) {
    if (holdStyle == HoldStyle::Hex) {
        return ConfigureHexAccent(accent, bounds, baseColor);
    }
    if (holdStyle == HoldStyle::Lightning) {
        return ConfigureLightningAccent(accent, bounds, baseColor);
    }
    if (holdStyle == HoldStyle::FluxField) {
        return ConfigureFluxFieldAccent(accent, bounds, baseColor);
    }
    if (holdStyle == HoldStyle::QuantumHalo) {
        return ConfigureQuantumHaloAccent(accent, bounds, baseColor);
    }
    return false;
}

} // namespace mousefx::macos_hold_pulse::detail
