#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx {

bool MacosInputIndicatorOverlay::Initialize() {
#if !defined(__APPLE__)
    return true;
#else
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    macos_input_indicator::RunOnMainThreadSync(^{
      NSPanel* panel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 72, 72)
                                                   styleMask:NSWindowStyleMaskBorderless
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
      if (panel == nil) {
          return;
      }
      macos_input_indicator_style::ConfigurePanel(panel);

      NSView* content = [panel contentView];
      macos_input_indicator_style::ConfigureContent(content);

      NSTextField* label = macos_input_indicator_style::CreateLabel(72);
      if (label == nil) {
          [panel release];
          return;
      }
      [content addSubview:label];

      panel_ = panel;
      labelField_ = label;
    });

    initialized_ = (panel_ != nullptr && labelField_ != nullptr);
    return initialized_;
#endif
}

void MacosInputIndicatorOverlay::Shutdown() {
#if !defined(__APPLE__)
    return;
#else
    displayGeneration_.fetch_add(1, std::memory_order_acq_rel);
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    macos_input_indicator::RunOnMainThreadSync(^{
      NSPanel* panel = (NSPanel*)panel_;
      NSTextField* label = (NSTextField*)labelField_;
      if (panel != nil) {
          [panel orderOut:nil];
      }
      if (label != nil) {
          [label removeFromSuperview];
          [label release];
      }
      if (panel != nil) {
          [panel release];
      }
      panel_ = nullptr;
      labelField_ = nullptr;
    });

    initialized_ = false;
#endif
}

} // namespace mousefx
