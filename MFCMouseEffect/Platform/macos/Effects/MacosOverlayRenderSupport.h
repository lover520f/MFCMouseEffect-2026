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
NSRect ClampOverlayFrameToScreenBounds(const NSRect& desiredFrame, const ScreenPoint& overlayPt);
CGFloat ResolveOverlayContentsScale(const ScreenPoint& overlayPt);
void ApplyOverlayContentScale(NSView* content, const ScreenPoint& overlayPt);
CGFloat ClampOverlayOpacity(CGFloat value);
CAAnimationGroup* CreateScaleFadeAnimationGroup(
    CGFloat fromScale,
    CGFloat toScale,
    CGFloat fromOpacity,
    CFTimeInterval duration);
#endif

} // namespace mousefx::macos_overlay_support
