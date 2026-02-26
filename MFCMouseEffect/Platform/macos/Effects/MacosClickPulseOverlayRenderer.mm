#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <cmath>

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
namespace {

std::string NormalizeClickType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "star" || value == "text") {
        return value;
    }
    return "ripple";
}

CGPathRef CreateStarPath(CGRect bounds, int points) {
    const int safePoints = (points < 4) ? 4 : points;
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    const CGFloat outerRadius = std::min(CGRectGetWidth(bounds), CGRectGetHeight(bounds)) * 0.42;
    const CGFloat innerRadius = outerRadius * 0.46;
    const CGFloat startAngle = -M_PI_2;

    CGMutablePathRef path = CGPathCreateMutable();
    for (int i = 0; i < safePoints * 2; ++i) {
        const CGFloat radius = (i % 2 == 0) ? outerRadius : innerRadius;
        const CGFloat angle = startAngle + static_cast<CGFloat>(i) * static_cast<CGFloat>(M_PI) / safePoints;
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

void ShowClickPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    (void)themeName;

    const std::string normalizedType = NormalizeClickType(effectType);
    const bool textMode = (normalizedType == "text");
    const bool starMode = (normalizedType == "star");

    const CGFloat size = textMode ? 152.0 : 138.0;
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
    base.opacity = 0.95;
    [content.layer addSublayer:base];

    if (starMode) {
        CAShapeLayer* star = [CAShapeLayer layer];
        star.frame = content.bounds;
        const CGRect starBounds = CGRectInset(content.bounds, 38.0, 38.0);
        CGPathRef starPath = CreateStarPath(starBounds, 5);
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
        text.opacity = 0.98;
        [content.layer addSublayer:text];
    }

    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = textMode ? @0.75 : @0.15;
    scale.toValue = @1.0;
    scale.duration = textMode ? 0.36 : 0.32;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @0.95;
    fade.toValue = @0.0;
    fade.duration = textMode ? 0.36 : 0.32;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = textMode ? 0.36 : 0.32;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [base addAnimation:group forKey:@"mfx_click_pulse"];

    RegisterClickPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(380) * NSEC_PER_MSEC),
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
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    return;
#else
    const ScreenPoint ptCopy = overlayPt;
    const MouseButton buttonCopy = button;
    const std::string typeCopy = effectType;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowClickPulseOverlayOnMain(ptCopy, buttonCopy, typeCopy, themeCopy);
    });
#endif
}

} // namespace mousefx::macos_click_pulse
