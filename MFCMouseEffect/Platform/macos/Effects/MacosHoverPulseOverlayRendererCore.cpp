#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
namespace {

NSWindow*& ActiveHoverWindow() {
    static NSWindow* window = nil;
    return window;
}

} // namespace
#endif

void CloseHoverPulseOverlayOnMain() {
#if !defined(__APPLE__)
    return;
#else
    NSWindow* window = ActiveHoverWindow();
    ActiveHoverWindow() = nil;
    if (window == nil) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(window));
#endif
}

void ShowHoverPulseOverlayOnMain(
    const HoverEffectRenderCommand& command,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    (void)themeName;
    CloseHoverPulseOverlayOnMain();

    const HoverPulseRenderPlan plan = BuildHoverPulseRenderPlan(command);

    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, command.overlayPoint);

    CAShapeLayer* ring = [CAShapeLayer layer];
    ConfigureHoverRingLayer(ring, content, plan);
    [content.layer addSublayer:ring];

    CABasicAnimation* breathe = [CABasicAnimation animationWithKeyPath:@"opacity"];
    breathe.fromValue = @0.25;
    breathe.toValue = @(macos_overlay_support::ResolveOverlayOpacity(command.baseOpacity, 0.05, 0.0));
    breathe.duration = plan.breatheDurationSec;
    breathe.autoreverses = YES;
    breathe.repeatCount = HUGE_VALF;
    [ring addAnimation:breathe forKey:@"mfx_hover_breathe"];

    AddHoverExtraLayersAndAnimations(content, plan);

    [window orderFrontRegardless];
    ActiveHoverWindow() = window;
#endif
}

void ShowHoverPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoverRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const HoverEffectProfile computeProfile{
        profile.sizePx,
        profile.breatheDurationSec,
        profile.spinDurationSec,
        profile.baseOpacity,
        profile.glowSizeScale,
        profile.tubesSizeScale,
        profile.glowBreatheScale,
        profile.tubesBreatheScale,
        profile.tubesSpinScale,
        {profile.colors.glowFillArgb, profile.colors.glowStrokeArgb, profile.colors.tubesStrokeArgb},
    };
    const HoverEffectRenderCommand command = ComputeHoverEffectRenderCommand(overlayPt, effectType, computeProfile);
    ShowHoverPulseOverlayOnMain(command, themeName);
#endif
}

size_t GetActiveHoverPulseWindowCountOnMain() {
#if !defined(__APPLE__)
    return 0;
#else
    return (ActiveHoverWindow() == nil) ? 0 : 1;
#endif
}

} // namespace mousefx::macos_hover_pulse
