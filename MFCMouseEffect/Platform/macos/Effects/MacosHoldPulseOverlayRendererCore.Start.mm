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
    (void)themeName;
    CloseHoldPulseOverlayOnMain();

    const std::string holdType = detail::NormalizeHoldType(effectType);
    const detail::HoldStyle holdStyle = detail::ResolveHoldStyle(holdType);
    const CGFloat size = static_cast<CGFloat>(profile.sizePx);
    const NSRect rawFrame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    const NSRect frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, overlayPt);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, overlayPt);

    NSColor* baseColor = detail::HoldBaseColor(button, holdStyle);

    CAShapeLayer* ring = [CAShapeLayer layer];
    ring.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 24.0, 24.0), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = [[baseColor colorWithAlphaComponent:0.16] CGColor];
    ring.strokeColor = [baseColor CGColor];
    ring.lineWidth = 2.4;
    ring.opacity = static_cast<float>(macos_overlay_support::ClampOverlayOpacity(profile.baseOpacity));
    [content.layer addSublayer:ring];

    CAShapeLayer* accent = [CAShapeLayer layer];
    accent.frame = content.bounds;
    detail::ConfigureHoldAccentLayer(accent, content.bounds, holdStyle, baseColor);
    accent.opacity = static_cast<float>(
        std::max<CGFloat>(0.1, macos_overlay_support::ClampOverlayOpacity(profile.baseOpacity - 0.06)));
    [content.layer addSublayer:accent];

    CABasicAnimation* breathe = [CABasicAnimation animationWithKeyPath:@"opacity"];
    breathe.fromValue = @0.35;
    breathe.toValue = @(macos_overlay_support::ClampOverlayOpacity(profile.baseOpacity + 0.03));
    breathe.duration = profile.breatheDurationSec;
    breathe.autoreverses = YES;
    breathe.repeatCount = HUGE_VALF;
    [ring addAnimation:breathe forKey:@"mfx_hold_breathe"];

    CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
    spin.fromValue = @0.0;
    spin.toValue = @(M_PI * 2.0);
    spin.duration = (holdStyle == detail::HoldStyle::QuantumHalo || holdStyle == detail::HoldStyle::FluxField)
        ? profile.rotateDurationFastSec
        : profile.rotateDurationSec;
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
    state.button = button;
#endif
}

} // namespace mousefx::macos_hold_pulse
