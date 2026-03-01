#pragma once

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#ifdef __OBJC__
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#else
struct objc_object;
using NSWindow = objc_object;
using NSView = objc_object;
using CAAnimationGroup = objc_object;
using NSRect = CGRect;
#endif
#endif

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

namespace mousefx::macos_overlay_support {

#if defined(__APPLE__)
using MainThreadCallback = void (*)(void*);
void RunOnMainThreadSync(MainThreadCallback callback, void* context);
void RunOnMainThreadAsync(MainThreadCallback callback, void* context);
#endif

#if defined(__APPLE__) && defined(__OBJC__)
void RunOnMainThreadSync(dispatch_block_t block);
void RunOnMainThreadAsync(dispatch_block_t block);
#endif

#if defined(__APPLE__)
NSWindow* CreateOverlayWindow(const NSRect& frame);
void ReleaseOverlayWindow(void* windowHandle);
void ShowOverlayWindow(void* windowHandle);
bool ResolveScreenFrameForPoint(const ScreenPoint& overlayPt, NSRect* frameOut);
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
