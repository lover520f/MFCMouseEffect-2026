#include "pch.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>

namespace mousefx::macos_overlay_support {

#if defined(__APPLE__)
namespace {

NSScreen* ResolveTargetScreen(const ScreenPoint& overlayPt) {
    NSArray<NSScreen*>* screens = [NSScreen screens];
    if (screens == nil || [screens count] == 0) {
        return nil;
    }
    const NSPoint point = NSMakePoint(static_cast<CGFloat>(overlayPt.x), static_cast<CGFloat>(overlayPt.y));
    for (NSScreen* screen in screens) {
        if (NSPointInRect(point, [screen frame])) {
            return screen;
        }
    }

    NSScreen* fallback = [NSScreen mainScreen];
    if (fallback != nil) {
        return fallback;
    }
    return [screens objectAtIndex:0];
}

CGFloat ClampCoordinate(CGFloat value, CGFloat minValue, CGFloat maxValue) {
    if (maxValue < minValue) {
        return minValue;
    }
    return std::clamp(value, minValue, maxValue);
}

} // namespace

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

NSWindow* CreateOverlayWindow(const NSRect& frame) {
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:NSWindowStyleMaskBorderless
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    if (window == nil) {
        return nil;
    }
    [window setOpaque:NO];
    [window setBackgroundColor:[NSColor clearColor]];
    [window setHasShadow:NO];
    [window setIgnoresMouseEvents:YES];
    [window setLevel:NSStatusWindowLevel];
    [window setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                   NSWindowCollectionBehaviorTransient)];
    return window;
}

NSRect ClampOverlayFrameToScreenBounds(const NSRect& desiredFrame, const ScreenPoint& overlayPt) {
    NSScreen* screen = ResolveTargetScreen(overlayPt);
    if (screen == nil) {
        return desiredFrame;
    }

    const NSRect bounds = [screen frame];
    if (bounds.size.width <= 0.0 || bounds.size.height <= 0.0 ||
        desiredFrame.size.width <= 0.0 || desiredFrame.size.height <= 0.0) {
        return desiredFrame;
    }
    // Keep effect anchor at the real input point. If the window goes partially
    // out of screen near edges, clipping is preferred over anchor drift.
    return desiredFrame;
}

CGFloat ResolveOverlayContentsScale(const ScreenPoint& overlayPt) {
    NSScreen* screen = ResolveTargetScreen(overlayPt);
    if (screen == nil) {
        return 1.0;
    }
    return ClampCoordinate([screen backingScaleFactor], 1.0, 4.0);
}

void ApplyOverlayContentScale(NSView* content, const ScreenPoint& overlayPt) {
    if (content == nil) {
        return;
    }
    [content setWantsLayer:YES];
    CALayer* root = [content layer];
    if (root == nil) {
        return;
    }
    const CGFloat scale = ResolveOverlayContentsScale(overlayPt);
    root.contentsScale = scale;
    for (CALayer* layer in [root sublayers]) {
        layer.contentsScale = scale;
    }
}

CGFloat ClampOverlayOpacity(CGFloat value) {
    return ClampCoordinate(value, 0.0, 1.0);
}

CGFloat ResolveOverlayOpacity(CGFloat baseOpacity, CGFloat delta, CGFloat minOpacity) {
    const CGFloat clamped = ClampOverlayOpacity(baseOpacity + delta);
    const CGFloat floor = ClampOverlayOpacity(minOpacity);
    return std::max(clamped, floor);
}

CAAnimationGroup* CreateScaleFadeAnimationGroup(
    CGFloat fromScale,
    CGFloat toScale,
    CGFloat fromOpacity,
    CFTimeInterval duration) {
    const CFTimeInterval clampedDuration = std::max<CFTimeInterval>(0.05, duration);
    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = @(fromScale);
    scale.toValue = @(toScale);
    scale.duration = clampedDuration;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @(ClampOverlayOpacity(fromOpacity));
    fade.toValue = @0.0;
    fade.duration = clampedDuration;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = clampedDuration;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    return group;
}

CGFloat ScaleOverlayMetric(
    CGFloat referenceSize,
    CGFloat baseValue,
    CGFloat baseReference,
    CGFloat minValue,
    CGFloat maxValue) {
    const CGFloat safeReference = std::max<CGFloat>(1.0, baseReference);
    const CGFloat safeSize = std::max<CGFloat>(1.0, referenceSize);
    const CGFloat scaled = baseValue * (safeSize / safeReference);
    return ClampCoordinate(scaled, minValue, maxValue);
}

CFTimeInterval ScaleOverlayDurationBySize(
    CFTimeInterval baseDuration,
    CGFloat overlaySize,
    CGFloat baseReference,
    CFTimeInterval minDuration,
    CFTimeInterval maxDuration) {
    const CGFloat factor = ScaleOverlayMetric(overlaySize, 1.0, baseReference, 0.88, 1.16);
    const CFTimeInterval scaled = std::max<CFTimeInterval>(0.04, baseDuration * static_cast<CFTimeInterval>(factor));
    return std::clamp<CFTimeInterval>(scaled, minDuration, maxDuration);
}
#endif

} // namespace mousefx::macos_overlay_support
