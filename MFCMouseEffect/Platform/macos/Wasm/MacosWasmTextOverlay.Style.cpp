#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmTextOverlay.Internal.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

namespace mousefx::platform::macos {

#if defined(__APPLE__)
void ConfigureWasmTextOverlayPanel(NSPanel* panel, CGFloat height) {
    [panel setOpaque:NO];
    [panel setBackgroundColor:[NSColor clearColor]];
    [panel setHasShadow:NO];
    [panel setIgnoresMouseEvents:YES];
    [panel setHidesOnDeactivate:NO];
    [panel setLevel:NSStatusWindowLevel];
    [panel setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                  NSWindowCollectionBehaviorTransient)];

    NSView* content = [panel contentView];
    [content setWantsLayer:YES];
    content.layer.backgroundColor = [[NSColor colorWithCalibratedWhite:0 alpha:0.58] CGColor];
    content.layer.cornerRadius = wasm_overlay_render_math::ClampFloat(height * 0.24, 8.0, 22.0);
    content.layer.borderWidth = 1.0;
    content.layer.borderColor = [[NSColor colorWithCalibratedWhite:1 alpha:0.22] CGColor];
}

NSTextField* CreateWasmTextOverlayLabel(
    CGFloat width,
    CGFloat height,
    CGFloat fontSize,
    uint32_t argb,
    NSString* value) {
    NSTextField* label = [[NSTextField alloc] initWithFrame:NSMakeRect(
        0,
        (height - fontSize - 6.0) * 0.5,
        width,
        fontSize + 6.0)];
    [label setEditable:NO];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setSelectable:NO];
    [label setAlignment:NSTextAlignmentCenter];
    [label setTextColor:wasm_overlay_render_math::ColorFromArgb(argb, 1.0)];
    [label setFont:[NSFont monospacedSystemFontOfSize:fontSize weight:NSFontWeightSemibold]];
    [label setStringValue:value];
    return label;
}

#endif

} // namespace mousefx::platform::macos
