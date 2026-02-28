#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

#include <algorithm>

namespace mousefx::macos_hold_pulse {

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

    NSColor* baseColor = detail::HoldBaseColor(command.button, holdStyle, profile);
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
    detail::ConfigureHoldAccentLayer(accent, content.bounds, holdStyle, baseColor);
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

    [window orderFrontRegardless];

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

} // namespace mousefx::macos_hold_pulse
