#include "pch.h"

#include "Platform/macos/Effects/MacosTextEffectFallback.h"

#include "MouseFx/Core/Diagnostics/TextEffectRuntimeDiagnostics.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Effects/MacosTextEffectFallbackSwiftBridge.h"
#include "Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.h"
#include "Settings/EmojiUtils.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <vector>

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#include <pthread.h>
#endif

namespace mousefx {

#if defined(__APPLE__)
namespace {

constexpr double kMinPanelSize = 200.0;

double ResolvePanelSize(double fontSize) {
    return std::max(kMinPanelSize, fontSize * 8.0);
}

struct ActivePanel final {
    void* handle = nullptr;
};

struct TextAnimationSpec final {
    ScreenPoint startPoint{};
    double durationMs = 300.0;
    double floatDistance = 48.0;
    double driftX = 0.0;
    double swayFreq = 1.0;
    double swayAmp = 6.0;
    double baseFontSize = 24.0;
    uint32_t argb = 0xFFFFFFFFu;
    std::string fontFamilyUtf8{};
    bool emojiText = false;
};

std::mutex& ActivePanelsMutex() {
    static std::mutex mutex;
    return mutex;
}

std::vector<ActivePanel>& ActivePanels() {
    static std::vector<ActivePanel> panels;
    return panels;
}

std::atomic<uint64_t>& AnimationGeneration() {
    static std::atomic<uint64_t> generation{1};
    return generation;
}

uint64_t MonotonicNowMs() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

void RunOnMainThreadAsync(dispatch_block_t block) {
    if (block == nullptr) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), block);
}

void RunOnMainThreadSync(dispatch_block_t block) {
    if (block == nullptr) {
        return;
    }
    if (pthread_main_np() != 0) {
        block();
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), block);
}

double Clamp01(double value) {
    return std::clamp(value, 0.0, 1.0);
}

double EaseOutCubic(double t) {
    const double u = 1.0 - Clamp01(t);
    return 1.0 - (u * u * u);
}

bool IsPanelTrackedLocked(void* panelHandle) {
    const auto& panels = ActivePanels();
    return std::any_of(
        panels.begin(),
        panels.end(),
        [panelHandle](const ActivePanel& item) { return item.handle == panelHandle; });
}

bool RemoveTrackedPanelLocked(void* panelHandle) {
    auto& panels = ActivePanels();
    const auto it = std::find_if(
        panels.begin(),
        panels.end(),
        [panelHandle](const ActivePanel& item) { return item.handle == panelHandle; });
    if (it == panels.end()) {
        return false;
    }
    panels.erase(it);
    return true;
}

void CloseTrackedPanel(void* panelHandle) {
    if (panelHandle == nullptr) {
        return;
    }

    bool removed = false;
    size_t remaining = 0;
    {
        std::lock_guard<std::mutex> lock(ActivePanelsMutex());
        removed = RemoveTrackedPanelLocked(panelHandle);
        if (removed) {
            remaining = ActivePanels().size();
        }
    }
    if (!removed) {
        return;
    }
    diagnostics::SetTextEffectFallbackActivePanels(remaining);
    mfx_macos_text_panel_release_v1(panelHandle);
}

void EnforceWindowCap(size_t maxConcurrentWindows) {
    if (maxConcurrentWindows == 0) {
        maxConcurrentWindows = 1;
    }

    std::vector<void*> toClose;
    size_t remaining = 0;
    {
        std::lock_guard<std::mutex> lock(ActivePanelsMutex());
        auto& panels = ActivePanels();
        while (panels.size() > maxConcurrentWindows) {
            if (panels.front().handle != nullptr) {
                toClose.push_back(panels.front().handle);
            }
            panels.erase(panels.begin());
        }
        remaining = panels.size();
    }
    diagnostics::SetTextEffectFallbackActivePanels(remaining);

    for (void* panelHandle : toClose) {
        mfx_macos_text_panel_release_v1(panelHandle);
    }
}

void ApplyTextFrame(void* panelHandle, const TextAnimationSpec& spec, double t) {
    if (panelHandle == nullptr) {
        return;
    }

    const double eased = EaseOutCubic(t);
    const double yOffset = eased * spec.floatDistance;
    const double xOffset = (t * spec.driftX) + std::sin(t * 3.1415926 * spec.swayFreq) * spec.swayAmp;

    double scale = 1.0;
    if (t < 0.3) {
        scale = 0.8 + (t / 0.3) * 0.4;
    } else {
        scale = 1.2 - ((t - 0.3) / 0.7) * 0.2;
    }

    double alphaFactor = 1.0;
    if (t < 0.08) {
        alphaFactor = 0.5 + (t / 0.08) * 0.5;
    } else if (t > 0.6) {
        alphaFactor = 1.0 - (t - 0.6) / 0.4;
    }
    alphaFactor = Clamp01(alphaFactor);

    const double panelSize = ResolvePanelSize(spec.baseFontSize);
    const double x = static_cast<double>(spec.startPoint.x) + xOffset - panelSize * 0.5;
    const double y = static_cast<double>(spec.startPoint.y) + yOffset - panelSize * 0.5;
    mfx_macos_text_panel_set_frame_v1(panelHandle, x, y, panelSize);

    const double fontSize = std::max(6.0, spec.baseFontSize * scale);
    const char* fontFamilyUtf8 = spec.fontFamilyUtf8.empty() ? "" : spec.fontFamilyUtf8.c_str();
    mfx_macos_text_panel_apply_style_v1(
        panelHandle,
        fontSize,
        spec.argb,
        alphaFactor,
        fontFamilyUtf8,
        spec.emojiText ? 1 : 0);
}

void StartTextAnimation(void* panelHandle, TextAnimationSpec spec) {
    if (panelHandle == nullptr) {
        return;
    }

    const uint64_t startTickMs = MonotonicNowMs();
    const uint64_t generation = AnimationGeneration().load(std::memory_order_acquire);

    auto step = std::make_shared<std::function<void()>>();
    *step = [panelHandle, spec, startTickMs, generation, step]() {
        if (generation != AnimationGeneration().load(std::memory_order_acquire)) {
            CloseTrackedPanel(panelHandle);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(ActivePanelsMutex());
            if (!IsPanelTrackedLocked(panelHandle)) {
                return;
            }
        }

        const uint64_t nowMs = MonotonicNowMs();
        const uint64_t elapsedMs = (nowMs >= startTickMs) ? (nowMs - startTickMs) : 0;
        const double t = Clamp01(static_cast<double>(elapsedMs) / std::max(1.0, spec.durationMs));
        ApplyTextFrame(panelHandle, spec, t);

        if (t >= 1.0) {
            CloseTrackedPanel(panelHandle);
            return;
        }

        dispatch_after(
            dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(16) * NSEC_PER_MSEC),
            dispatch_get_main_queue(),
            ^{
              if (step) {
                  (*step)();
              }
            });
    };

    (*step)();
}

} // namespace
#endif

bool MacosTextEffectFallback::EnsureInitialized(size_t count) {
    maxConcurrentWindows_ = std::clamp<size_t>(count, 1, 48);
    return true;
}

void MacosTextEffectFallback::Shutdown() {
#if !defined(__APPLE__)
    return;
#else
    AnimationGeneration().fetch_add(1, std::memory_order_acq_rel);
    RunOnMainThreadSync(^{
      std::vector<void*> toClose;
      {
          std::lock_guard<std::mutex> lock(ActivePanelsMutex());
          auto& panels = ActivePanels();
          toClose.reserve(panels.size());
          for (const auto& item : panels) {
              if (item.handle != nullptr) {
                  toClose.push_back(item.handle);
              }
          }
          panels.clear();
      }
      diagnostics::SetTextEffectFallbackActivePanels(0);
      for (void* panelHandle : toClose) {
          mfx_macos_text_panel_release_v1(panelHandle);
      }
    });
#endif
}

void MacosTextEffectFallback::ShowText(
    const ScreenPoint& pt,
    const std::wstring& text,
    Argb color,
    const TextConfig& config) {
#if !defined(__APPLE__)
    (void)pt;
    (void)text;
    (void)color;
    (void)config;
    return;
#else
    if (text.empty()) {
        diagnostics::RecordTextEffectFallbackError("empty_text");
        return;
    }
    if (maxConcurrentWindows_ == 0) {
        maxConcurrentWindows_ = 8;
    }

    diagnostics::RecordTextEffectFallbackShow(pt, text);

    const std::string utf8Text = Utf16ToUtf8(text.c_str());
    if (utf8Text.empty()) {
        diagnostics::RecordTextEffectFallbackError("utf8_empty");
        return;
    }

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> driftXDist(-50, 49);
    std::uniform_int_distribution<int> swayFreqDist(0, 199);
    std::uniform_int_distribution<int> swayAmpDist(0, 99);

    TextAnimationSpec spec{};
    ScreenPoint overlayPt = pt;
    ScreenPoint cocoaPt{};
    if (macos_overlay_coord_conversion::TryConvertQuartzToCocoa(pt, &cocoaPt)) {
        overlayPt = cocoaPt;
    }
    spec.startPoint = overlayPt;
    spec.durationMs = static_cast<double>(std::max(config.durationMs, 1));
    spec.floatDistance = static_cast<double>(std::max(config.floatDistance, 0));
    spec.driftX = static_cast<double>(driftXDist(rng));
    spec.swayFreq = 1.0 + static_cast<double>(swayFreqDist(rng)) / 100.0;
    spec.swayAmp = 5.0 + static_cast<double>(swayAmpDist(rng)) / 10.0;
    const double baseFontPx = static_cast<double>(config.fontSize) * (96.0 / 72.0);
    spec.baseFontSize = std::max(baseFontPx, 6.0);
    spec.argb = color.value;
    spec.fontFamilyUtf8 = Utf16ToUtf8(config.fontFamily.c_str());
    spec.emojiText = settings::HasEmojiStarter(text);

    const size_t cap = maxConcurrentWindows_;
    RunOnMainThreadAsync(^{
      const double panelSize = ResolvePanelSize(spec.baseFontSize);
      const char* fontFamilyUtf8 = spec.fontFamilyUtf8.empty() ? "" : spec.fontFamilyUtf8.c_str();
      void* panelHandle = mfx_macos_text_panel_create_v1(
          utf8Text.c_str(),
          panelSize,
          spec.baseFontSize,
          spec.argb,
          fontFamilyUtf8,
          spec.emojiText ? 1 : 0);
      if (panelHandle == nullptr) {
          diagnostics::RecordTextEffectFallbackError("panel_nil");
          return;
      }

      {
          std::lock_guard<std::mutex> lock(ActivePanelsMutex());
          ActivePanels().push_back(ActivePanel{panelHandle});
          diagnostics::SetTextEffectFallbackActivePanels(ActivePanels().size());
      }
      EnforceWindowCap(cap);

      mfx_macos_text_panel_show_v1(panelHandle);
      diagnostics::RecordTextEffectFallbackPanelCreated();
      StartTextAnimation(panelHandle, spec);
    });
#endif
}

} // namespace mousefx
