#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx {

void MacosInputIndicatorOverlay::Hide() {
#if !defined(__APPLE__)
    return;
#else
    displayGeneration_.fetch_add(1, std::memory_order_acq_rel);
    macos_input_indicator::RunOnMainThreadAsync(^{
      macos_input_indicator_style::HidePanel(panel_);
    });
#endif
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
      if (panel_ == nullptr) {
          return;
      }
      macos_input_indicator_style::ApplyPanelPresentation(
          panel_, plan.x, plan.y, plan.sizePx, plan.durationMs, labelCopy);
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
            macos_input_indicator_style::HidePanel(panel_);
          });
    });
#else
    (void)generation;
#endif
}

} // namespace mousefx
