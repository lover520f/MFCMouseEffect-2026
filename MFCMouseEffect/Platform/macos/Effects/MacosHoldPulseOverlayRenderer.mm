#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>
#include <cmath>

namespace mousefx::macos_hold_pulse {

#if defined(__APPLE__)
namespace {

enum class HoldStyle {
    Charge,
    Lightning,
    Hex,
    TechRing,
    Hologram,
    Neon,
    QuantumHalo,
    FluxField,
};

struct HoldOverlayState {
    NSWindow* window = nil;
    CAShapeLayer* ring = nil;
    CAShapeLayer* accent = nil;
    macos_effect_profile::HoldRenderProfile profile{};
    HoldStyle style = HoldStyle::Charge;
    std::string effectType{};
    MouseButton button = MouseButton::Left;
};

HoldOverlayState& State() {
    static HoldOverlayState state;
    return state;
}

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

void CloseHoldPulseOverlayOnMain() {
    HoldOverlayState& state = State();
    if (state.window == nil) {
        return;
    }
    [state.window orderOut:nil];
    [state.window release];
    state.window = nil;
    state.ring = nil;
    state.accent = nil;
    state.effectType.clear();
}

void StartHoldPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoldRenderProfile& profile) {
    (void)themeName;
    CloseHoldPulseOverlayOnMain();

    const std::string holdType = NormalizeHoldType(effectType);
    const HoldStyle holdStyle = ResolveHoldStyle(holdType);
    const CGFloat size = static_cast<CGFloat>(profile.sizePx);
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    NSColor* baseColor = HoldBaseColor(button, holdStyle);

    CAShapeLayer* ring = [CAShapeLayer layer];
    ring.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 24.0, 24.0), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = [[baseColor colorWithAlphaComponent:0.16] CGColor];
    ring.strokeColor = [baseColor CGColor];
    ring.lineWidth = 2.4;
    ring.opacity = static_cast<float>(profile.baseOpacity);
    [content.layer addSublayer:ring];

    CAShapeLayer* accent = [CAShapeLayer layer];
    accent.frame = content.bounds;
    if (holdStyle == HoldStyle::Hex) {
        CGPathRef path = CreateHexPath(CGRectInset(content.bounds, 38.0, 38.0));
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 1.8;
    } else if (holdStyle == HoldStyle::Lightning) {
        CGPathRef path = CreateLightningPath(CGRectInset(content.bounds, 36.0, 36.0));
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [baseColor CGColor];
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 1.0;
    } else if (holdStyle == HoldStyle::FluxField) {
        CGPathRef path = CreateFluxFieldPath(CGRectInset(content.bounds, 36.0, 36.0));
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 2.0;
    } else if (holdStyle == HoldStyle::QuantumHalo) {
        CGPathRef path = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 36.0, 36.0), nullptr);
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = 2.2;
    } else {
        CGPathRef path = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 44.0, 44.0), nullptr);
        accent.path = path;
        CGPathRelease(path);
        accent.fillColor = [NSColor clearColor].CGColor;
        accent.strokeColor = [[baseColor colorWithAlphaComponent:0.85] CGColor];
        accent.lineWidth = 1.4;
        accent.lineDashPattern = @[@6, @6];
    }
    accent.opacity = static_cast<float>(std::max(0.1, profile.baseOpacity - 0.06));
    [content.layer addSublayer:accent];

    CABasicAnimation* breathe = [CABasicAnimation animationWithKeyPath:@"opacity"];
    breathe.fromValue = @0.35;
    breathe.toValue = @(std::min(1.0, profile.baseOpacity + 0.03));
    breathe.duration = profile.breatheDurationSec;
    breathe.autoreverses = YES;
    breathe.repeatCount = HUGE_VALF;
    [ring addAnimation:breathe forKey:@"mfx_hold_breathe"];

    CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
    spin.fromValue = @0.0;
    spin.toValue = @(M_PI * 2.0);
    spin.duration = (holdStyle == HoldStyle::QuantumHalo || holdStyle == HoldStyle::FluxField)
        ? profile.rotateDurationFastSec
        : profile.rotateDurationSec;
    spin.repeatCount = HUGE_VALF;
    [accent addAnimation:spin forKey:@"mfx_hold_spin"];

    [window orderFrontRegardless];

    HoldOverlayState& state = State();
    state.window = window;
    state.ring = ring;
    state.accent = accent;
    state.profile = profile;
    state.style = holdStyle;
    state.effectType = holdType;
    state.button = button;
}

void UpdateHoldPulseOverlayOnMain(const ScreenPoint& overlayPt, uint32_t holdMs) {
    HoldOverlayState& state = State();
    if (state.window == nil || state.ring == nil) {
        return;
    }

    const NSRect frame = [state.window frame];
    const CGFloat w = frame.size.width;
    const CGFloat h = frame.size.height;
    [state.window setFrameOrigin:NSMakePoint(overlayPt.x - w * 0.5, overlayPt.y - h * 0.5)];

    const CGFloat progress = std::min<CGFloat>(
        1.0,
        static_cast<CGFloat>(holdMs) / std::max<CGFloat>(1.0f, static_cast<CGFloat>(state.profile.progressFullMs)));
    const CGFloat scale = 1.0 + progress * 0.20;
    state.ring.transform = CATransform3DMakeScale(scale, scale, 1.0);
    state.ring.lineWidth = 2.4 + progress * 1.4;
    const CGFloat baseOpacity = static_cast<CGFloat>(state.profile.baseOpacity);
    state.ring.opacity = std::max<CGFloat>(0.2, baseOpacity - 0.18f + progress * 0.20f);

    if (state.accent != nil) {
        state.accent.opacity = std::max<CGFloat>(0.15, baseOpacity - 0.35f + progress * 0.35f);
    }
}

} // namespace
#endif

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const ScreenPoint ptCopy = overlayPt;
    const MouseButton buttonCopy = button;
    const std::string typeCopy = effectType;
    const std::string themeCopy = themeName;
    const macos_effect_profile::HoldRenderProfile profileCopy = profile;
    macos_overlay_support::RunOnMainThreadAsync(^{
      StartHoldPulseOverlayOnMain(ptCopy, buttonCopy, typeCopy, themeCopy, profileCopy);
    });
#endif
}

void UpdateHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    uint32_t holdMs,
    const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)holdMs;
    (void)profile;
    return;
#else
    const ScreenPoint ptCopy = overlayPt;
    macos_overlay_support::RunOnMainThreadAsync(^{
      (void)profile;
      UpdateHoldPulseOverlayOnMain(ptCopy, holdMs);
    });
#endif
}

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    StartHoldPulseOverlay(overlayPt, button, effectType, themeName, macos_effect_profile::DefaultHoldRenderProfile());
}

void UpdateHoldPulseOverlay(const ScreenPoint& overlayPt, uint32_t holdMs) {
    UpdateHoldPulseOverlay(overlayPt, holdMs, macos_effect_profile::DefaultHoldRenderProfile());
}

void StopHoldPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseHoldPulseOverlayOnMain();
    });
#endif
}

size_t GetActiveHoldPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    __block size_t count = 0;
    macos_overlay_support::RunOnMainThreadSync(^{
      count = (State().window == nil) ? 0 : 1;
    });
    return count;
#endif
}

} // namespace mousefx::macos_hold_pulse
