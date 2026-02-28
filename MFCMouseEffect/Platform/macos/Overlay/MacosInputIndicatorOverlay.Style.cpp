#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx::macos_input_indicator_style {

#if defined(__APPLE__)

void ConfigurePanel(NSPanel* panel) {
    if (panel == nil) {
        return;
    }
    [panel setOpaque:NO];
    [panel setBackgroundColor:[NSColor clearColor]];
    [panel setHasShadow:NO];
    [panel setIgnoresMouseEvents:YES];
    [panel setHidesOnDeactivate:NO];
    [panel setLevel:NSStatusWindowLevel];
    [panel setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                  NSWindowCollectionBehaviorTransient)];
}

void ConfigureContent(NSView* content) {
    if (content == nil) {
        return;
    }
    [content setWantsLayer:YES];
    content.layer.backgroundColor = [[NSColor colorWithCalibratedWhite:0 alpha:0.58] CGColor];
    content.layer.cornerRadius = 14.0;
    content.layer.borderWidth = 1.0;
    content.layer.borderColor = [[NSColor colorWithCalibratedWhite:1 alpha:0.25] CGColor];
}

NSTextField* CreateLabel(int sizePx) {
    const CGFloat width = static_cast<CGFloat>(sizePx);
    NSTextField* label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, (width - 32.0) * 0.5, width, 32)];
    if (label == nil) {
        return nil;
    }
    [label setEditable:NO];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setSelectable:NO];
    [label setAlignment:NSTextAlignmentCenter];
    [label setTextColor:[NSColor colorWithCalibratedRed:0.84 green:0.95 blue:1 alpha:1]];
    [label setFont:[NSFont monospacedSystemFontOfSize:15 weight:NSFontWeightSemibold]];
    [label setStringValue:@""];
    return label;
}

void ApplyPanelPresentation(NSPanel* panel, NSTextField* label, int x, int y, int sizePx, const std::string& text) {
    if (panel == nil || label == nil) {
        return;
    }

    const NSRect frame = NSMakeRect(
        static_cast<CGFloat>(x),
        static_cast<CGFloat>(y),
        static_cast<CGFloat>(sizePx),
        static_cast<CGFloat>(sizePx));
    [panel setFrame:frame display:NO];

    NSView* content = [panel contentView];
    if (content != nil) {
        content.layer.cornerRadius = static_cast<CGFloat>(sizePx) * 0.22;
    }

    [label setFrame:NSMakeRect(0, (sizePx - 32) * 0.5, sizePx, 32)];
    [label setStringValue:macos_input_indicator::NsStringFromUtf8(text)];
    [panel setAlphaValue:1.0];
    [panel orderFrontRegardless];
}

#endif

} // namespace mousefx::macos_input_indicator_style
