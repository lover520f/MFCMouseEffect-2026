#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>
#include <cmath>

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
namespace {

std::string NormalizeScrollType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "helix" || value == "twinkle") {
        return value;
    }
    return "arrow";
}

void ShowScrollPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    if (delta == 0) {
        return;
    }
    (void)themeName;

    const std::string normalizedType = NormalizeScrollType(effectType);
    const bool helixMode = (normalizedType == "helix");
    const bool twinkleMode = (normalizedType == "twinkle");

    int strengthLevel = static_cast<int>(std::abs(delta) / 120);
    if (strengthLevel < 1) {
        strengthLevel = 1;
    }
    if (strengthLevel > 6) {
        strengthLevel = 6;
    }

    const CGFloat size = horizontal ? static_cast<CGFloat>(profile.horizontalSizePx) : static_cast<CGFloat>(profile.verticalSizePx);
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    const CGFloat bodyThickness = 18.0;
    const CGFloat bodyLength = 56.0 + static_cast<CGFloat>(strengthLevel) * 8.0;
    const CGRect bodyRect = horizontal
        ? CGRectMake((size - bodyLength) * 0.5, (size - bodyThickness) * 0.5, bodyLength, bodyThickness)
        : CGRectMake((size - bodyThickness) * 0.5, (size - bodyLength) * 0.5, bodyThickness, bodyLength);

    CAShapeLayer* body = [CAShapeLayer layer];
    body.frame = content.bounds;
    CGPathRef bodyPath = CGPathCreateWithRoundedRect(bodyRect, bodyThickness * 0.5, bodyThickness * 0.5, nullptr);
    body.path = bodyPath;
    CGPathRelease(bodyPath);
    body.fillColor = [ScrollPulseFillColor(horizontal, delta) CGColor];
    body.strokeColor = [ScrollPulseStrokeColor(horizontal, delta) CGColor];
    body.lineWidth = 2.0;
    body.opacity = static_cast<float>(profile.baseOpacity);
    [content.layer addSublayer:body];

    CAShapeLayer* arrow = [CAShapeLayer layer];
    arrow.frame = content.bounds;
    CGPathRef arrowPath = CreateScrollPulseDirectionArrowPath(bodyRect, horizontal, delta);
    arrow.path = arrowPath;
    CGPathRelease(arrowPath);
    arrow.fillColor = [ScrollPulseStrokeColor(horizontal, delta) CGColor];
    arrow.opacity = static_cast<float>(std::min(1.0, profile.baseOpacity + 0.02));
    [content.layer addSublayer:arrow];

    if (helixMode) {
        CAShapeLayer* helix = [CAShapeLayer layer];
        helix.frame = content.bounds;
        const CGRect helixRect = CGRectInset(bodyRect, -9.0, -9.0);
        CGPathRef helixPath = CGPathCreateWithEllipseInRect(helixRect, nullptr);
        helix.path = helixPath;
        CGPathRelease(helixPath);
        helix.fillColor = [NSColor clearColor].CGColor;
        helix.strokeColor = [ScrollPulseStrokeColor(horizontal, -delta) CGColor];
        helix.lineWidth = 1.6;
        helix.opacity = 0.82;
        [content.layer addSublayer:helix];

        CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
        spin.fromValue = @0.0;
        spin.toValue = @(M_PI * 1.5);
        spin.duration = 0.45;
        spin.repeatCount = 1;
        [helix addAnimation:spin forKey:@"mfx_scroll_helix_spin"];
    }

    if (twinkleMode) {
        CAShapeLayer* twinkle = [CAShapeLayer layer];
        twinkle.frame = content.bounds;
        const CGRect twinkleRect = CGRectInset(bodyRect, -20.0, -20.0);
        CGPathRef twinklePath = CGPathCreateWithEllipseInRect(twinkleRect, nullptr);
        twinkle.path = twinklePath;
        CGPathRelease(twinklePath);
        twinkle.fillColor = [NSColor clearColor].CGColor;
        twinkle.strokeColor = [ScrollPulseStrokeColor(horizontal, delta) CGColor];
        twinkle.lineWidth = 1.0;
        twinkle.opacity = 0.55;
        [content.layer addSublayer:twinkle];
    }

    const CFTimeInterval duration = profile.baseDurationSec + static_cast<CFTimeInterval>(strengthLevel) * profile.perStrengthStepSec;
    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = @0.72;
    scale.toValue = @1.04;
    scale.duration = duration;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @(std::min(1.0, profile.baseOpacity + 0.02));
    fade.toValue = @0.0;
    fade.duration = duration;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = duration;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [body addAnimation:group forKey:@"mfx_scroll_body_pulse"];
    [arrow addAnimation:group forKey:@"mfx_scroll_arrow_pulse"];

    RegisterScrollPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    const int closeAfterMs = static_cast<int>(duration * 1000.0) + profile.closePaddingMs;
    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeScrollPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
}

} // namespace
#endif

void CloseAllScrollPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseAllScrollPulseWindowsNow();
    });
#endif
}

void ShowScrollPulseOverlay(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ScrollRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)horizontal;
    (void)delta;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    if (delta == 0) {
        return;
    }

    const ScreenPoint ptCopy = overlayPt;
    const bool horizontalCopy = horizontal;
    const int deltaCopy = delta;
    const std::string typeCopy = effectType;
    const std::string themeCopy = themeName;
    const macos_effect_profile::ScrollRenderProfile profileCopy = profile;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowScrollPulseOverlayOnMain(ptCopy, horizontalCopy, deltaCopy, typeCopy, themeCopy, profileCopy);
    });
#endif
}

void ShowScrollPulseOverlay(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName) {
    ShowScrollPulseOverlay(overlayPt, horizontal, delta, effectType, themeName, macos_effect_profile::DefaultScrollRenderProfile());
}

} // namespace mousefx::macos_scroll_pulse
