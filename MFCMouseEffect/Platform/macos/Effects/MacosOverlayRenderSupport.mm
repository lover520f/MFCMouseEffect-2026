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

    NSRect clamped = desiredFrame;
    if (clamped.size.width > bounds.size.width) {
        clamped.size.width = bounds.size.width;
    }
    if (clamped.size.height > bounds.size.height) {
        clamped.size.height = bounds.size.height;
    }

    const CGFloat minX = NSMinX(bounds);
    const CGFloat maxX = NSMaxX(bounds) - clamped.size.width;
    const CGFloat minY = NSMinY(bounds);
    const CGFloat maxY = NSMaxY(bounds) - clamped.size.height;
    clamped.origin.x = ClampCoordinate(clamped.origin.x, minX, maxX);
    clamped.origin.y = ClampCoordinate(clamped.origin.y, minY, maxY);
    return clamped;
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
#endif

} // namespace mousefx::macos_overlay_support
