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

bool BuildHexAccentPath(CGRect bounds, CGPathRef* pathOut, CGFloat* lineWidthOut, bool* fillWithBaseColorOut) {
    *pathOut = CreateHexPath(CGRectInset(bounds, 38.0, 38.0));
    *lineWidthOut = 1.8;
    *fillWithBaseColorOut = false;
    return true;
}

bool BuildLightningAccentPath(CGRect bounds, CGPathRef* pathOut, CGFloat* lineWidthOut, bool* fillWithBaseColorOut) {
    *pathOut = CreateLightningPath(CGRectInset(bounds, 36.0, 36.0));
    *lineWidthOut = 1.0;
    *fillWithBaseColorOut = true;
    return true;
}

bool BuildFluxFieldAccentPath(CGRect bounds, CGPathRef* pathOut, CGFloat* lineWidthOut, bool* fillWithBaseColorOut) {
    *pathOut = CreateFluxFieldPath(CGRectInset(bounds, 36.0, 36.0));
    *lineWidthOut = 2.0;
    *fillWithBaseColorOut = false;
    return true;
}

bool BuildQuantumHaloAccentPath(CGRect bounds, CGPathRef* pathOut, CGFloat* lineWidthOut, bool* fillWithBaseColorOut) {
    *pathOut = CGPathCreateWithEllipseInRect(CGRectInset(bounds, 36.0, 36.0), nullptr);
    *lineWidthOut = 2.2;
    *fillWithBaseColorOut = false;
    return true;
}

} // namespace

bool BuildSpecialHoldAccentPath(
    CGRect bounds,
    HoldStyle holdStyle,
    CGPathRef* pathOut,
    CGFloat* lineWidthOut,
    bool* fillWithBaseColorOut) {
    if (pathOut == nullptr || lineWidthOut == nullptr || fillWithBaseColorOut == nullptr) {
        return false;
    }
    if (holdStyle == HoldStyle::Hex) {
        return BuildHexAccentPath(bounds, pathOut, lineWidthOut, fillWithBaseColorOut);
    }
    if (holdStyle == HoldStyle::Lightning) {
        return BuildLightningAccentPath(bounds, pathOut, lineWidthOut, fillWithBaseColorOut);
    }
    if (holdStyle == HoldStyle::FluxField) {
        return BuildFluxFieldAccentPath(bounds, pathOut, lineWidthOut, fillWithBaseColorOut);
    }
    if (holdStyle == HoldStyle::QuantumHalo) {
        return BuildQuantumHaloAccentPath(bounds, pathOut, lineWidthOut, fillWithBaseColorOut);
    }
    return false;
}

} // namespace mousefx::macos_hold_pulse::detail
