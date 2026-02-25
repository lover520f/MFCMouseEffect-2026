#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <cmath>

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
namespace {

void RunOnMainThreadSync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    if ([NSThread isMainThread]) {
        block();
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), block);
}

void RunOnMainThreadAsync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), block);
}

void ShowScrollPulseOverlayOnMain(const ScreenPoint& overlayPt, bool horizontal, int delta, const std::string& themeName) {
    int strengthLevel = static_cast<int>(std::abs(delta) / 120);
    if (strengthLevel < 1) {
        strengthLevel = 1;
    }
    if (strengthLevel > 6) {
        strengthLevel = 6;
    }
    const std::string theme = themeName;
    (void)theme;

    const CGFloat size = horizontal ? 148.0 : 138.0;
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:NSWindowStyleMaskBorderless
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    if (window == nil) {
        return;
    }
    [window setOpaque:NO];
    [window setBackgroundColor:[NSColor clearColor]];
    [window setHasShadow:NO];
    [window setIgnoresMouseEvents:YES];
    [window setLevel:NSStatusWindowLevel];
    [window setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                   NSWindowCollectionBehaviorTransient)];

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
    body.opacity = 0.96;
    [content.layer addSublayer:body];

    CAShapeLayer* arrow = [CAShapeLayer layer];
    arrow.frame = content.bounds;
    CGPathRef arrowPath = CreateScrollPulseDirectionArrowPath(bodyRect, horizontal, delta);
    arrow.path = arrowPath;
    CGPathRelease(arrowPath);
    arrow.fillColor = [ScrollPulseStrokeColor(horizontal, delta) CGColor];
    arrow.opacity = 0.98;
    [content.layer addSublayer:arrow];

    const CFTimeInterval duration = 0.28 + static_cast<CFTimeInterval>(strengthLevel) * 0.018;
    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = @0.72;
    scale.toValue = @1.04;
    scale.duration = duration;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @0.98;
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

    const int closeAfterMs = static_cast<int>(duration * 1000.0) + 70;
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
    RunOnMainThreadSync(^{
      CloseAllScrollPulseWindowsNow();
    });
#endif
}

void ShowScrollPulseOverlay(const ScreenPoint& overlayPt, bool horizontal, int delta, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)horizontal;
    (void)delta;
    (void)themeName;
    return;
#else
    if (delta == 0) {
        return;
    }

    const ScreenPoint ptCopy = overlayPt;
    const bool horizontalCopy = horizontal;
    const int deltaCopy = delta;
    const std::string themeCopy = themeName;
    RunOnMainThreadAsync(^{
      ShowScrollPulseOverlayOnMain(ptCopy, horizontalCopy, deltaCopy, themeCopy);
    });
#endif
}

} // namespace mousefx::macos_scroll_pulse
