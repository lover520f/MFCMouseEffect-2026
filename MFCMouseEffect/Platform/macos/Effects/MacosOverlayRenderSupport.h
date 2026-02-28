#pragma once

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

namespace mousefx::macos_overlay_support {

#if defined(__APPLE__)
void RunOnMainThreadSync(dispatch_block_t block);
void RunOnMainThreadAsync(dispatch_block_t block);
NSWindow* CreateOverlayWindow(const NSRect& frame);
void ReleaseOverlayWindow(void* windowHandle);
NSRect ClampOverlayFrameToScreenBounds(const NSRect& desiredFrame, const ScreenPoint& overlayPt);
CGFloat ResolveOverlayContentsScale(const ScreenPoint& overlayPt);
void ApplyOverlayContentScale(NSView* content, const ScreenPoint& overlayPt);
CGFloat ClampOverlayOpacity(CGFloat value);
CGFloat ResolveOverlayOpacity(CGFloat baseOpacity, CGFloat delta, CGFloat minOpacity);
CAAnimationGroup* CreateScaleFadeAnimationGroup(
    CGFloat fromScale,
    CGFloat toScale,
    CGFloat fromOpacity,
    CFTimeInterval duration);
CGFloat ScaleOverlayMetric(
    CGFloat referenceSize,
    CGFloat baseValue,
    CGFloat baseReference,
    CGFloat minValue,
    CGFloat maxValue);
CFTimeInterval ScaleOverlayDurationBySize(
    CFTimeInterval baseDuration,
    CGFloat overlaySize,
    CGFloat baseReference,
    CFTimeInterval minDuration,
    CFTimeInterval maxDuration);
#endif

} // namespace mousefx::macos_overlay_support
