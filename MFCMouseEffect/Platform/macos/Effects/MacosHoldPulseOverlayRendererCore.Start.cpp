#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

#include <algorithm>

namespace mousefx::macos_hold_pulse {

namespace {

#if defined(__APPLE__)
NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

NSColor* HoldBaseColor(
    MouseButton button,
    detail::HoldStyle style,
    const macos_effect_profile::HoldRenderProfile& profile) {
    if (style == detail::HoldStyle::Lightning) {
        return ArgbToNsColor(profile.colors.lightningStrokeArgb);
    }
    if (style == detail::HoldStyle::Hex) {
        return ArgbToNsColor(profile.colors.hexStrokeArgb);
    }
    if (style == detail::HoldStyle::Hologram) {
        return ArgbToNsColor(profile.colors.hologramStrokeArgb);
    }
    if (style == detail::HoldStyle::QuantumHalo) {
        return ArgbToNsColor(profile.colors.quantumHaloStrokeArgb);
    }
    if (style == detail::HoldStyle::FluxField) {
        return ArgbToNsColor(profile.colors.fluxFieldStrokeArgb);
    }
    if (style == detail::HoldStyle::TechRing || style == detail::HoldStyle::Neon) {
        return ArgbToNsColor(profile.colors.techNeonStrokeArgb);
    }
    if (button == MouseButton::Right) {
        return ArgbToNsColor(profile.colors.rightBaseStrokeArgb);
    }
    if (button == MouseButton::Middle) {
        return ArgbToNsColor(profile.colors.middleBaseStrokeArgb);
    }
    return ArgbToNsColor(profile.colors.leftBaseStrokeArgb);
}

void ConfigureHoldAccentLayer(CAShapeLayer* accent, CGRect bounds, detail::HoldStyle holdStyle, NSColor* baseColor) {
    CGPathRef specialPath = nullptr;
    CGFloat specialLineWidth = 0.0;
    bool fillWithBaseColor = false;
    if (detail::BuildSpecialHoldAccentPath(
            bounds,
            holdStyle,
            &specialPath,
            &specialLineWidth,
            &fillWithBaseColor)) {
        accent.path = specialPath;
        CGPathRelease(specialPath);
        accent.fillColor = fillWithBaseColor ? [baseColor CGColor] : [NSColor clearColor].CGColor;
        accent.strokeColor = [baseColor CGColor];
        accent.lineWidth = specialLineWidth;
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

} // namespace

void StartHoldPulseOverlayOnMain(const HoldEffectStartCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    (void)themeName;
    CloseHoldPulseOverlayOnMain();

    const std::string holdType = detail::NormalizeHoldType(command.normalizedType);
    const detail::HoldStyle holdStyle = detail::ResolveHoldStyle(holdType);
    const CGFloat size = static_cast<CGFloat>(command.sizePx);
    const NSRect rawFrame =
        NSMakeRect(command.overlayPoint.x - size * 0.5, command.overlayPoint.y - size * 0.5, size, size);
    const NSRect frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, command.overlayPoint);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, command.overlayPoint);

    macos_effect_profile::HoldRenderProfile profile{};
    profile.sizePx = command.sizePx;
    profile.progressFullMs = command.progressFullMs;
    profile.breatheDurationSec = command.breatheDurationSec;
    profile.rotateDurationSec = command.rotateDurationSec;
    profile.rotateDurationFastSec = command.rotateDurationFastSec;
    profile.baseOpacity = command.baseOpacity;
    profile.colors.leftBaseStrokeArgb = command.colors.leftBaseStrokeArgb;
    profile.colors.rightBaseStrokeArgb = command.colors.rightBaseStrokeArgb;
    profile.colors.middleBaseStrokeArgb = command.colors.middleBaseStrokeArgb;
    profile.colors.lightningStrokeArgb = command.colors.lightningStrokeArgb;
    profile.colors.hexStrokeArgb = command.colors.hexStrokeArgb;
    profile.colors.hologramStrokeArgb = command.colors.hologramStrokeArgb;
    profile.colors.quantumHaloStrokeArgb = command.colors.quantumHaloStrokeArgb;
    profile.colors.fluxFieldStrokeArgb = command.colors.fluxFieldStrokeArgb;
    profile.colors.techNeonStrokeArgb = command.colors.techNeonStrokeArgb;

    NSColor* baseColor = HoldBaseColor(command.button, holdStyle, profile);
    const CGFloat ringInset = macos_overlay_support::ScaleOverlayMetric(size, 24.0, 160.0, 10.0, 44.0);
    const CGFloat ringLineWidth = macos_overlay_support::ScaleOverlayMetric(size, 2.4, 160.0, 1.2, 4.8);

    CAShapeLayer* ring = [CAShapeLayer layer];
    ring.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, ringInset, ringInset), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = [[baseColor colorWithAlphaComponent:0.16] CGColor];
    ring.strokeColor = [baseColor CGColor];
    ring.lineWidth = ringLineWidth;
    ring.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(command.baseOpacity, 0.0, 0.0));
    [content.layer addSublayer:ring];

    CAShapeLayer* accent = [CAShapeLayer layer];
    accent.frame = content.bounds;
    ConfigureHoldAccentLayer(accent, content.bounds, holdStyle, baseColor);
    accent.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(command.baseOpacity, -0.06, 0.1));
    [content.layer addSublayer:accent];

    CABasicAnimation* breathe = [CABasicAnimation animationWithKeyPath:@"opacity"];
    breathe.fromValue = @0.35;
    breathe.toValue = @(macos_overlay_support::ResolveOverlayOpacity(command.baseOpacity, 0.03, 0.0));
    breathe.duration = command.breatheDurationSec;
    breathe.autoreverses = YES;
    breathe.repeatCount = HUGE_VALF;
    [ring addAnimation:breathe forKey:@"mfx_hold_breathe"];

    CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
    spin.fromValue = @0.0;
    spin.toValue = @(M_PI * 2.0);
    spin.duration = (holdStyle == detail::HoldStyle::QuantumHalo || holdStyle == detail::HoldStyle::FluxField)
        ? command.rotateDurationFastSec
        : command.rotateDurationSec;
    spin.repeatCount = HUGE_VALF;
    [accent addAnimation:spin forKey:@"mfx_hold_spin"];

    macos_overlay_support::ShowOverlayWindow(reinterpret_cast<void*>(window));

    detail::HoldOverlayState& state = detail::State();
    state.window = window;
    state.ring = ring;
    state.accent = accent;
    state.profile = profile;
    state.style = holdStyle;
    state.effectType = holdType;
    state.button = command.button;
#endif
}

void StartHoldPulseOverlayOnMain(
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
    HoldEffectStartCommand command{};
    command.overlayPoint = overlayPt;
    command.button = button;
    command.normalizedType = effectType;
    command.sizePx = profile.sizePx;
    command.progressFullMs = profile.progressFullMs;
    command.breatheDurationSec = profile.breatheDurationSec;
    command.rotateDurationSec = profile.rotateDurationSec;
    command.rotateDurationFastSec = profile.rotateDurationFastSec;
    command.baseOpacity = profile.baseOpacity;
    command.colors.leftBaseStrokeArgb = profile.colors.leftBaseStrokeArgb;
    command.colors.rightBaseStrokeArgb = profile.colors.rightBaseStrokeArgb;
    command.colors.middleBaseStrokeArgb = profile.colors.middleBaseStrokeArgb;
    command.colors.lightningStrokeArgb = profile.colors.lightningStrokeArgb;
    command.colors.hexStrokeArgb = profile.colors.hexStrokeArgb;
    command.colors.hologramStrokeArgb = profile.colors.hologramStrokeArgb;
    command.colors.quantumHaloStrokeArgb = profile.colors.quantumHaloStrokeArgb;
    command.colors.fluxFieldStrokeArgb = profile.colors.fluxFieldStrokeArgb;
    command.colors.techNeonStrokeArgb = profile.colors.techNeonStrokeArgb;
    StartHoldPulseOverlayOnMain(command, themeName);
#endif
}

void UpdateHoldPulseOverlayOnMain(const ScreenPoint& overlayPt, uint32_t holdMs) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)holdMs;
    return;
#else
    detail::HoldOverlayState& state = detail::State();
    if (state.window == nil || state.ring == nil) {
        return;
    }

    const NSRect frame = [state.window frame];
    const CGFloat w = frame.size.width;
    const CGFloat h = frame.size.height;
    const NSRect rawFrame = NSMakeRect(overlayPt.x - w * 0.5, overlayPt.y - h * 0.5, w, h);
    const NSRect clampedFrame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, overlayPt);
    [state.window setFrameOrigin:clampedFrame.origin];
    macos_overlay_support::ApplyOverlayContentScale([state.window contentView], overlayPt);

    const CGFloat progress = std::min<CGFloat>(
        1.0,
        static_cast<CGFloat>(holdMs) / std::max<CGFloat>(1.0f, static_cast<CGFloat>(state.profile.progressFullMs)));
    const CGFloat scale = 1.0 + progress * 0.20;
    const CGFloat baseLineWidth = macos_overlay_support::ScaleOverlayMetric(w, 2.4, 160.0, 1.2, 4.8);
    const CGFloat progressLineDelta = macos_overlay_support::ScaleOverlayMetric(w, 1.4, 160.0, 0.7, 3.0);
    state.ring.transform = CATransform3DMakeScale(scale, scale, 1.0);
    state.ring.lineWidth = baseLineWidth + progress * progressLineDelta;
    const CGFloat baseOpacity = static_cast<CGFloat>(state.profile.baseOpacity);
    state.ring.opacity = macos_overlay_support::ResolveOverlayOpacity(
        baseOpacity,
        -0.18f + progress * 0.20f,
        0.2);

    if (state.accent != nil) {
        state.accent.opacity = macos_overlay_support::ResolveOverlayOpacity(
            baseOpacity,
            -0.35f + progress * 0.35f,
            0.15);
    }
#endif
}

size_t GetActiveHoldPulseWindowCountOnMain() {
#if !defined(__APPLE__)
    return 0;
#else
    return (detail::State().window == nil) ? 0 : 1;
#endif
}

} // namespace mousefx::macos_hold_pulse
