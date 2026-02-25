#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx {

MacosInputIndicatorOverlay::~MacosInputIndicatorOverlay() {
    Shutdown();
}

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
      content.layer.cornerRadius = 14.0;
      content.layer.borderWidth = 1.0;
      content.layer.borderColor = [[NSColor colorWithCalibratedWhite:1 alpha:0.25] CGColor];

      NSTextField* label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 20, 72, 32)];
      [label setEditable:NO];
      [label setBezeled:NO];
      [label setDrawsBackground:NO];
      [label setSelectable:NO];
      [label setAlignment:NSTextAlignmentCenter];
      [label setTextColor:[NSColor colorWithCalibratedRed:0.84 green:0.95 blue:1 alpha:1]];
      [label setFont:[NSFont monospacedSystemFontOfSize:15 weight:NSFontWeightSemibold]];
      [label setStringValue:@""];
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

void MacosInputIndicatorOverlay::Hide() {
#if !defined(__APPLE__)
    return;
#else
    displayGeneration_.fetch_add(1, std::memory_order_acq_rel);
    macos_input_indicator::RunOnMainThreadAsync(^{
      NSPanel* panel = (NSPanel*)panel_;
      if (panel != nil) {
          [panel orderOut:nil];
      }
    });
#endif
}

void MacosInputIndicatorOverlay::UpdateConfig(const InputIndicatorConfig& cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = cfg;
}

void MacosInputIndicatorOverlay::ShowAt(ScreenPoint pt, const std::string& label) {
    InputIndicatorConfig cfg{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_ || !config_.enabled) {
            return;
        }
        cfg = config_;
    }

    const int sizePx = macos_input_indicator::ClampInt(cfg.sizePx, 28, 220);
    const int durationMs = macos_input_indicator::ClampInt(cfg.durationMs, 80, 5000);
    const bool absolute = (ToLowerAscii(TrimAscii(cfg.positionMode)) == "absolute");
    const ScreenPoint overlayPt = absolute ? pt : ScreenToOverlayPoint(pt);
    const int x = absolute ? cfg.absoluteX : (overlayPt.x + cfg.offsetX);
    const int y = absolute ? cfg.absoluteY : (overlayPt.y + cfg.offsetY);
    const uint64_t generation = displayGeneration_.fetch_add(1, std::memory_order_acq_rel) + 1;
    const std::string labelCopy = label;

#if defined(__APPLE__)
    macos_input_indicator::RunOnMainThreadAsync(^{
      NSPanel* panel = (NSPanel*)panel_;
      NSTextField* text = (NSTextField*)labelField_;
      if (panel == nil || text == nil) {
          return;
      }

      const NSRect frame = NSMakeRect(static_cast<CGFloat>(x), static_cast<CGFloat>(y), sizePx, sizePx);
      [panel setFrame:frame display:NO];

      NSView* content = [panel contentView];
      if (content != nil) {
          content.layer.cornerRadius = static_cast<CGFloat>(sizePx) * 0.22;
      }
      [text setFrame:NSMakeRect(0, (sizePx - 32) * 0.5, sizePx, 32)];
      [text setStringValue:macos_input_indicator::NsStringFromUtf8(labelCopy)];
      {
          std::lock_guard<std::mutex> debugLock(debugMutex_);
          lastAppliedLabel_ = labelCopy;
          applyCount_++;
      }
      [panel setAlphaValue:1.0];
      [panel orderFrontRegardless];

      dispatch_after(
          dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(durationMs) * NSEC_PER_MSEC),
          dispatch_get_main_queue(),
          ^{
            if (displayGeneration_.load(std::memory_order_acquire) != generation) {
                return;
            }
            [panel orderOut:nil];
          });
    });
#else
    (void)durationMs;
    (void)generation;
#endif
}

bool MacosInputIndicatorOverlay::ShouldShowKeyboard() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && config_.enabled && config_.keyboardEnabled;
}

} // namespace mousefx
