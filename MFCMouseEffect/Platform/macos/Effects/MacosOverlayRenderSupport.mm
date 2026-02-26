#include "pch.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_overlay_support {

#if defined(__APPLE__)
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
#endif

} // namespace mousefx::macos_overlay_support
