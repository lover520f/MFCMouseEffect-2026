#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <cmath>

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
namespace {

std::string NormalizeTrailType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "line" ||
        value == "particle" ||
        value == "meteor" ||
        value == "streamer" ||
        value == "electric" ||
        value == "tubes") {
        return value;
    }
    return "line";
}

NSColor* TrailStrokeColor(const std::string& trailType) {
    if (trailType == "meteor") {
        return [NSColor colorWithCalibratedRed:1.0 green:0.64 blue:0.30 alpha:0.95];
    }
    if (trailType == "streamer") {
        return [NSColor colorWithCalibratedRed:0.32 green:0.95 blue:0.92 alpha:0.95];
    }
    if (trailType == "electric") {
        return [NSColor colorWithCalibratedRed:0.58 green:0.73 blue:1.0 alpha:0.95];
    }
    if (trailType == "tubes") {
        return [NSColor colorWithCalibratedRed:0.43 green:0.88 blue:0.52 alpha:0.95];
    }
    if (trailType == "particle") {
        return [NSColor colorWithCalibratedRed:1.0 green:0.84 blue:0.34 alpha:0.95];
    }
    return [NSColor colorWithCalibratedRed:0.40 green:0.76 blue:1.0 alpha:0.95];
}

NSColor* TrailFillColor(const std::string& trailType) {
    NSColor* stroke = TrailStrokeColor(trailType);
    return [stroke colorWithAlphaComponent:0.24];
}

CGPathRef CreateTrailLinePath(CGRect bounds, double deltaX, double deltaY, const std::string& trailType) {
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    const CGFloat len = (trailType == "particle") ? 10.0 : 20.0;
    const CGFloat width = (trailType == "particle") ? 2.0 : 3.0;

    CGFloat dx = static_cast<CGFloat>(deltaX);
    CGFloat dy = static_cast<CGFloat>(deltaY);
    const CGFloat norm = std::sqrt(dx * dx + dy * dy);
    if (norm < 0.0001) {
        dx = 1.0;
        dy = 0.0;
    } else {
        dx /= norm;
        dy /= norm;
    }

    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, nullptr, cx - dx * len, cy - dy * len);
    if (trailType == "electric") {
        CGPathAddLineToPoint(path, nullptr, cx - dy * width, cy + dx * width);
        CGPathAddLineToPoint(path, nullptr, cx + dy * width, cy - dx * width);
    }
    CGPathAddLineToPoint(path, nullptr, cx + dx * len, cy + dy * len);
    return path;
}

void ShowTrailPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName) {
    (void)themeName;
    const std::string trailType = NormalizeTrailType(effectType);

    const CGFloat size = (trailType == "particle") ? 48.0 : 64.0;
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    CAShapeLayer* core = [CAShapeLayer layer];
    core.frame = content.bounds;

    if (trailType == "tubes") {
        CGPathRef outer = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 9.0, 9.0), nullptr);
        core.path = outer;
        CGPathRelease(outer);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [TrailStrokeColor(trailType) CGColor];
        core.lineWidth = 3.2;
    } else if (trailType == "particle") {
        CGPathRef dot = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 16.0, 16.0), nullptr);
        core.path = dot;
        CGPathRelease(dot);
        core.fillColor = [TrailStrokeColor(trailType) CGColor];
        core.strokeColor = [TrailStrokeColor(trailType) CGColor];
        core.lineWidth = 1.2;
    } else {
        CGPathRef line = CreateTrailLinePath(content.bounds, deltaX, deltaY, trailType);
        core.path = line;
        CGPathRelease(line);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [TrailStrokeColor(trailType) CGColor];
        core.lineCap = kCALineCapRound;
        core.lineJoin = kCALineJoinRound;
        core.lineWidth = (trailType == "meteor") ? 4.0 : 3.0;
    }

    core.opacity = 0.95;
    [content.layer addSublayer:core];

    if (trailType == "meteor" || trailType == "streamer") {
        CAShapeLayer* glow = [CAShapeLayer layer];
        glow.frame = content.bounds;
        CGPathRef glowPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 18.0, 18.0), nullptr);
        glow.path = glowPath;
        CGPathRelease(glowPath);
        glow.fillColor = [TrailFillColor(trailType) CGColor];
        glow.strokeColor = [NSColor clearColor].CGColor;
        glow.opacity = 0.85;
        [content.layer addSublayer:glow];
    }

    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = @0.65;
    scale.toValue = @1.0;
    scale.duration = 0.22;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @0.95;
    fade.toValue = @0.0;
    fade.duration = 0.22;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = 0.22;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [core addAnimation:group forKey:@"mfx_trail_pulse"];

    RegisterTrailPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(260) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeTrailPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
}

} // namespace
#endif

void CloseAllTrailPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseAllTrailPulseWindowsNow();
    });
#endif
}

void ShowTrailPulseOverlay(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)deltaX;
    (void)deltaY;
    (void)effectType;
    (void)themeName;
    return;
#else
    const ScreenPoint ptCopy = overlayPt;
    const double dxCopy = deltaX;
    const double dyCopy = deltaY;
    const std::string typeCopy = effectType;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowTrailPulseOverlayOnMain(ptCopy, dxCopy, dyCopy, typeCopy, themeCopy);
    });
#endif
}

} // namespace mousefx::macos_trail_pulse
