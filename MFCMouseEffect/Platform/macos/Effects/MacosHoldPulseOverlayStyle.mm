#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#include <algorithm>
#include <cmath>
#endif

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
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

} // namespace

std::string NormalizeHoldType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty()) {
        return "charge";
    }
    return value;
}

HoldStyle ResolveHoldStyle(const std::string& holdType) {
    if (holdType.find("lightning") != std::string::npos) {
        return HoldStyle::Lightning;
    }
    if (holdType.find("hex") != std::string::npos) {
        return HoldStyle::Hex;
    }
    if (holdType.find("hologram") != std::string::npos) {
        return HoldStyle::Hologram;
    }
    if (holdType.find("tech") != std::string::npos) {
        return HoldStyle::TechRing;
    }
    if (holdType.find("neon") != std::string::npos) {
        return HoldStyle::Neon;
    }
    if (holdType.find("quantum_halo") != std::string::npos) {
        return HoldStyle::QuantumHalo;
    }
    if (holdType.find("flux_field") != std::string::npos) {
        return HoldStyle::FluxField;
    }
    return HoldStyle::Charge;
}

NSColor* HoldBaseColor(MouseButton button, HoldStyle style) {
    if (style == HoldStyle::Lightning) {
        return [NSColor colorWithCalibratedRed:0.56 green:0.73 blue:1.0 alpha:0.96];
    }
    if (style == HoldStyle::Hex) {
        return [NSColor colorWithCalibratedRed:0.44 green:0.90 blue:0.60 alpha:0.96];
    }
    if (style == HoldStyle::Hologram) {
        return [NSColor colorWithCalibratedRed:0.42 green:0.95 blue:0.90 alpha:0.96];
    }
    if (style == HoldStyle::QuantumHalo) {
        return [NSColor colorWithCalibratedRed:0.66 green:0.70 blue:1.0 alpha:0.96];
    }
    if (style == HoldStyle::FluxField) {
        return [NSColor colorWithCalibratedRed:0.45 green:0.95 blue:0.62 alpha:0.96];
    }
    if (style == HoldStyle::TechRing || style == HoldStyle::Neon) {
        return [NSColor colorWithCalibratedRed:0.50 green:0.78 blue:1.0 alpha:0.96];
    }
    if (button == MouseButton::Right) {
        return [NSColor colorWithCalibratedRed:1.0 green:0.62 blue:0.26 alpha:0.96];
    }
    if (button == MouseButton::Middle) {
        return [NSColor colorWithCalibratedRed:0.42 green:0.88 blue:0.54 alpha:0.96];
    }
    return [NSColor colorWithCalibratedRed:0.26 green:0.74 blue:1.0 alpha:0.96];
}

void ConfigureHoldAccentLayer(CAShapeLayer* accent, CGRect bounds, HoldStyle holdStyle, NSColor* baseColor) {
    if (holdStyle == HoldStyle::Hex) {
        CGPathRef path = CreateHexPath(CGRectInset(bounds, 38.0, 38.0));
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 1.8;
        return;
    }
    if (holdStyle == HoldStyle::Lightning) {
        CGPathRef path = CreateLightningPath(CGRectInset(bounds, 36.0, 36.0));
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [baseColor CGColor];
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 1.0;
        return;
    }
    if (holdStyle == HoldStyle::FluxField) {
        CGPathRef path = CreateFluxFieldPath(CGRectInset(bounds, 36.0, 36.0));
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 2.0;
        return;
    }
    if (holdStyle == HoldStyle::QuantumHalo) {
        CGPathRef path = CGPathCreateWithEllipseInRect(CGRectInset(bounds, 36.0, 36.0), nullptr);
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 2.2;
        return;
    }

    CGPathRef path = CGPathCreateWithEllipseInRect(CGRectInset(bounds, 44.0, 44.0), nullptr);
    accent.path = path;
    CGPathRelease(path);
    accent.fillColor = [NSColor clearColor].CGColor;
    accent.strokeColor = [[baseColor colorWithAlphaComponent:0.85] CGColor];
    accent.lineWidth = 1.4;
    accent.lineDashPattern = @[@6, @6];
}
#endif

} // namespace mousefx::macos_hold_pulse::detail
