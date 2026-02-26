#pragma once

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_overlay_support {

#if defined(__APPLE__)
void RunOnMainThreadSync(dispatch_block_t block);
void RunOnMainThreadAsync(dispatch_block_t block);
NSWindow* CreateOverlayWindow(const NSRect& frame);
#endif

} // namespace mousefx::macos_overlay_support
