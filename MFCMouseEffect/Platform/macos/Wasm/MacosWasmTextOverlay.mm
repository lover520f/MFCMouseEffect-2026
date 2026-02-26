#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::platform::macos {

namespace {

#if defined(__APPLE__)
#endif

} // namespace

WasmOverlayRenderResult ShowWasmTextOverlay(
    const ScreenPoint& screenPt,
    const std::wstring& text,
    uint32_t argb,
    float scale,
    uint32_t lifeMs) {
#if !defined(__APPLE__)
    (void)screenPt;
    (void)text;
    (void)argb;
    (void)scale;
    (void)lifeMs;
    return WasmOverlayRenderResult::Failed;
#else
    if (text.empty()) {
        return WasmOverlayRenderResult::Failed;
    }

    const std::string utf8Text = Utf16ToUtf8(text.c_str());
    if (utf8Text.empty()) {
        return WasmOverlayRenderResult::Failed;
    }

    const ScreenPoint pt = ScreenToOverlayPoint(screenPt);
    const uint32_t durationMs = wasm_overlay_render_math::ClampLifeMs(lifeMs);
    const CGFloat textScale = wasm_overlay_render_math::ClampScale(scale);
    const CGFloat fontSize = wasm_overlay_render_math::ClampFloat(13.0 * textScale, 9.0, 42.0);
    const CGFloat width = wasm_overlay_render_math::ClampFloat(18.0 + static_cast<CGFloat>(utf8Text.size()) * (fontSize * 0.72), 64.0, 460.0);
    const CGFloat height = wasm_overlay_render_math::ClampFloat(fontSize * 2.0, 30.0, 88.0);
    const WasmOverlayAdmissionResult admission = TryAcquireWasmOverlaySlot(WasmOverlayKind::Text);
    if (admission != WasmOverlayAdmissionResult::Accepted) {
        return (admission == WasmOverlayAdmissionResult::RejectedByCapacity)
            ? WasmOverlayRenderResult::ThrottledByCapacity
            : WasmOverlayRenderResult::ThrottledByInterval;
    }

    RunWasmOverlayOnMainThreadAsync([=] {
      NSString* value = [NSString stringWithUTF8String:utf8Text.c_str()];
      if (value == nil) {
          ReleaseWasmOverlaySlot();
          return;
      }

      NSRect frame = NSMakeRect(pt.x - width * 0.5, pt.y - height - 24.0, width, height);
      NSPanel* panel = [[NSPanel alloc] initWithContentRect:frame
                                                   styleMask:NSWindowStyleMaskBorderless
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
      if (panel == nil) {
          ReleaseWasmOverlaySlot();
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

      NSView* content = [panel contentView];
      [content setWantsLayer:YES];
      content.layer.backgroundColor = [[NSColor colorWithCalibratedWhite:0 alpha:0.58] CGColor];
      content.layer.cornerRadius = wasm_overlay_render_math::ClampFloat(height * 0.24, 8.0, 22.0);
      content.layer.borderWidth = 1.0;
      content.layer.borderColor = [[NSColor colorWithCalibratedWhite:1 alpha:0.22] CGColor];

      NSTextField* label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, (height - fontSize - 6.0) * 0.5, width, fontSize + 6.0)];
      [label setEditable:NO];
      [label setBezeled:NO];
      [label setDrawsBackground:NO];
      [label setSelectable:NO];
      [label setAlignment:NSTextAlignmentCenter];
      [label setTextColor:wasm_overlay_render_math::ColorFromArgb(argb, 1.0)];
      [label setFont:[NSFont monospacedSystemFontOfSize:fontSize weight:NSFontWeightSemibold]];
      [label setStringValue:value];
      [content addSubview:label];
      [label release];

      RegisterWasmOverlayWindow(reinterpret_cast<void*>(panel));
      [panel orderFrontRegardless];

      dispatch_after(
          dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(durationMs) * NSEC_PER_MSEC),
          dispatch_get_main_queue(),
          ^{
            if (!TakeWasmOverlayWindow(reinterpret_cast<void*>(panel))) {
                return;
            }
            [panel orderOut:nil];
            [panel release];
          });
    });
    return WasmOverlayRenderResult::Rendered;
#endif
}

} // namespace mousefx::platform::macos
