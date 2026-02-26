#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>

namespace mousefx::macos_hold_pulse {

#if defined(__APPLE__)
namespace {

struct HoldOverlayState {
    NSWindow* window = nil;
    CAShapeLayer* ring = nil;
    CAShapeLayer* accent = nil;
    macos_effect_profile::HoldRenderProfile profile{};
    detail::HoldStyle style = detail::HoldStyle::Charge;
    std::string effectType{};
    MouseButton button = MouseButton::Left;
};

HoldOverlayState& State() {
    static HoldOverlayState state;
    return state;
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

    const std::string holdType = detail::NormalizeHoldType(effectType);
    const detail::HoldStyle holdStyle = detail::ResolveHoldStyle(holdType);
    const CGFloat size = static_cast<CGFloat>(profile.sizePx);
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    NSColor* baseColor = detail::HoldBaseColor(button, holdStyle);

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
    detail::ConfigureHoldAccentLayer(accent, content.bounds, holdStyle, baseColor);
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
    spin.duration = (holdStyle == detail::HoldStyle::QuantumHalo || holdStyle == detail::HoldStyle::FluxField)
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
