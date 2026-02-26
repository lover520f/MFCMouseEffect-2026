#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
namespace {

void ShowClickPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile) {
    (void)themeName;

    const std::string normalizedType = NormalizeClickType(effectType);
    const bool textMode = (normalizedType == "text");
    const bool starMode = (normalizedType == "star");

    const CGFloat size = textMode ? static_cast<CGFloat>(profile.textSizePx) : static_cast<CGFloat>(profile.normalSizePx);
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    CAShapeLayer* base = [CAShapeLayer layer];
    base.frame = content.bounds;
    const CGFloat inset = textMode ? 12.0 : 18.0;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(
        CGRectMake(inset, inset, size - inset * 2.0, size - inset * 2.0),
        nullptr);
    base.path = ringPath;
    CGPathRelease(ringPath);
    base.fillColor = [ClickPulseFillColor(button) CGColor];
    base.strokeColor = [ClickPulseStrokeColor(button) CGColor];
    base.lineWidth = textMode ? 2.1 : 2.4;
    base.opacity = static_cast<float>(profile.baseOpacity);
    [content.layer addSublayer:base];

    if (starMode) {
        CAShapeLayer* star = [CAShapeLayer layer];
        star.frame = content.bounds;
        const CGRect starBounds = CGRectInset(content.bounds, 38.0, 38.0);
        CGPathRef starPath = CreateClickPulseStarPath(starBounds, 5);
        star.path = starPath;
        CGPathRelease(starPath);
        star.fillColor = [ClickPulseStrokeColor(button) CGColor];
        star.strokeColor = [ClickPulseStrokeColor(button) CGColor];
        star.lineWidth = 1.0;
        star.opacity = 0.98;
        [content.layer addSublayer:star];
    }

    if (textMode) {
        CATextLayer* text = [CATextLayer layer];
        text.frame = CGRectMake(0.0, size * 0.30, size, 36.0);
        text.alignmentMode = kCAAlignmentCenter;
        text.foregroundColor = [ClickPulseStrokeColor(button) CGColor];
        text.contentsScale = [NSScreen mainScreen].backingScaleFactor;
        text.fontSize = 24.0;
        text.font = (__bridge CFTypeRef)[NSFont boldSystemFontOfSize:24.0];
        switch (button) {
        case MouseButton::Right:
            text.string = @"RIGHT";
            break;
        case MouseButton::Middle:
            text.string = @"MIDDLE";
            break;
        case MouseButton::Left:
        default:
            text.string = @"LEFT";
            break;
        }
        text.opacity = static_cast<float>(std::min(1.0, profile.baseOpacity + 0.03));
        [content.layer addSublayer:text];
    }

    const CFTimeInterval animationDuration = textMode ? profile.textDurationSec : profile.normalDurationSec;
    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = textMode ? @0.75 : @0.15;
    scale.toValue = @1.0;
    scale.duration = animationDuration;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @(profile.baseOpacity);
    fade.toValue = @0.0;
    fade.duration = animationDuration;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = animationDuration;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [base addAnimation:group forKey:@"mfx_click_pulse"];

    RegisterClickPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(
            DISPATCH_TIME_NOW,
            static_cast<int64_t>(static_cast<int>(animationDuration * 1000.0) + profile.closePaddingMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeClickPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
}

} // namespace
#endif

void CloseAllClickPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseAllClickPulseWindowsNow();
    });
#endif
}

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile) {
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
    const macos_effect_profile::ClickRenderProfile profileCopy = profile;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowClickPulseOverlayOnMain(ptCopy, buttonCopy, typeCopy, themeCopy, profileCopy);
    });
#endif
}

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    ShowClickPulseOverlay(overlayPt, button, effectType, themeName, macos_effect_profile::DefaultClickRenderProfile());
}

} // namespace mousefx::macos_click_pulse
