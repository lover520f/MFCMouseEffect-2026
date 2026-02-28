#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"
#include "Platform/macos/Wasm/MacosWasmTextOverlay.Internal.h"

#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::platform::macos {

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

    const WasmTextOverlayLayout layout = BuildWasmTextOverlayLayout(screenPt, utf8Text.size(), scale, lifeMs);
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

      NSPanel* panel = [[NSPanel alloc] initWithContentRect:layout.frame
                                                   styleMask:NSWindowStyleMaskBorderless
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
      if (panel == nil) {
          ReleaseWasmOverlaySlot();
          return;
      }

      ConfigureWasmTextOverlayPanel(panel, layout.height);

      NSView* content = [panel contentView];
      NSTextField* label = CreateWasmTextOverlayLabel(layout.width, layout.height, layout.fontSize, argb, value);
      [content addSubview:label];
      [label release];

      RegisterWasmOverlayWindow(reinterpret_cast<void*>(panel));
      [panel orderFrontRegardless];

      dispatch_after(
          dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(layout.durationMs) * NSEC_PER_MSEC),
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
