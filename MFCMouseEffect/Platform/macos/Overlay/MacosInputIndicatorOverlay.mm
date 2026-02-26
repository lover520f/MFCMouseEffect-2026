#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"
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

    const macos_input_indicator_detail::IndicatorShowPlan plan =
        macos_input_indicator_detail::BuildIndicatorShowPlan(cfg, pt);
    const uint64_t generation = displayGeneration_.fetch_add(1, std::memory_order_acq_rel) + 1;
    const std::string labelCopy = label;

#if defined(__APPLE__)
    macos_input_indicator::RunOnMainThreadAsync(^{
      NSPanel* panel = (NSPanel*)panel_;
      NSTextField* text = (NSTextField*)labelField_;
      if (panel == nil || text == nil) {
          return;
      }
      macos_input_indicator_style::ApplyPanelPresentation(
          panel, text, plan.x, plan.y, plan.sizePx, labelCopy);
      {
          std::lock_guard<std::mutex> debugLock(debugMutex_);
          lastAppliedLabel_ = labelCopy;
          applyCount_++;
      }

      dispatch_after(
          dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.durationMs) * NSEC_PER_MSEC),
          dispatch_get_main_queue(),
          ^{
            if (displayGeneration_.load(std::memory_order_acquire) != generation) {
                return;
            }
            [panel orderOut:nil];
          });
    });
#else
    (void)generation;
#endif
}

bool MacosInputIndicatorOverlay::ShouldShowKeyboard() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && config_.enabled && config_.keyboardEnabled;
}

} // namespace mousefx
