#include "pch.h"

#include "Platform/macos/Effects/MacosTextEffectFallback.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Core/Diagnostics/TextEffectRuntimeDiagnostics.h"
#include "Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.h"
#include "MouseFx/Utils/StringUtils.h"
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
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx {

#if defined(__APPLE__)
namespace {

constexpr double kMinPanelSize = 200.0;

double ResolvePanelSize(double fontSize) {
    return std::max(kMinPanelSize, fontSize * 8.0);
}

struct ActivePanel final {
    NSPanel* panel = nil;
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
    static std::vector<ActivePanel> windows;
    return windows;
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
    if ([NSThread isMainThread]) {
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

NSColor* ColorFromArgb(uint32_t argb, double alphaScale) {
    const CGFloat baseAlpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat alpha = static_cast<CGFloat>(Clamp01(baseAlpha * alphaScale));
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

NSString* NsStringFromUtf8(const std::string& value) {
    if (value.empty()) {
        return nil;
    }
    return [NSString stringWithUTF8String:value.c_str()];
}

NSFont* ResolveLabelFont(const TextAnimationSpec& spec, double fontSize) {
    const CGFloat size = static_cast<CGFloat>(std::max(6.0, fontSize));
    if (spec.emojiText) {
        NSFont* emoji = [NSFont fontWithName:@"Apple Color Emoji" size:size];
        if (emoji != nil) {
            return emoji;
        }
        return [NSFont systemFontOfSize:size weight:NSFontWeightRegular];
    }

    NSString* preferred = NsStringFromUtf8(spec.fontFamilyUtf8);
    if (preferred != nil) {
        NSFont* custom = [NSFont fontWithName:preferred size:size];
        if (custom != nil) {
            return custom;
        }
    }
    return [NSFont boldSystemFontOfSize:size];
}

bool IsPanelTrackedLocked(NSPanel* panel) {
    const auto& panels = ActivePanels();
    return std::any_of(
        panels.begin(),
        panels.end(),
        [panel](const ActivePanel& item) { return item.panel == panel; });
}

bool RemoveTrackedPanelLocked(NSPanel* panel) {
    auto& panels = ActivePanels();
    const auto it = std::find_if(
        panels.begin(),
        panels.end(),
        [panel](const ActivePanel& item) { return item.panel == panel; });
    if (it == panels.end()) {
        return false;
    }
    panels.erase(it);
    return true;
}

void CloseTrackedPanel(NSPanel* panel) {
    if (panel == nil) {
        return;
    }

    bool removed = false;
    size_t remaining = 0;
    {
        std::lock_guard<std::mutex> lock(ActivePanelsMutex());
        removed = RemoveTrackedPanelLocked(panel);
        if (removed) {
            remaining = ActivePanels().size();
        }
    }
    if (!removed) {
        return;
    }
    diagnostics::SetTextEffectFallbackActivePanels(remaining);
    [panel orderOut:nil];
    [panel release];
}

void EnforceWindowCap(size_t maxConcurrentWindows) {
    if (maxConcurrentWindows == 0) {
        maxConcurrentWindows = 1;
    }

    std::vector<NSPanel*> toClose;
    size_t remaining = 0;
    {
        std::lock_guard<std::mutex> lock(ActivePanelsMutex());
        auto& panels = ActivePanels();
        while (panels.size() > maxConcurrentWindows) {
            if (panels.front().panel != nil) {
                toClose.push_back(panels.front().panel);
            }
            panels.erase(panels.begin());
        }
        remaining = panels.size();
    }
    diagnostics::SetTextEffectFallbackActivePanels(remaining);

    for (NSPanel* panel : toClose) {
        [panel orderOut:nil];
        [panel release];
    }
}

void ApplyTextFrame(NSPanel* panel, CATextLayer* label, const TextAnimationSpec& spec, double t) {
    if (panel == nil || label == nil) {
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
    const CGFloat x = static_cast<CGFloat>(spec.startPoint.x + xOffset - panelSize * 0.5);
    const CGFloat y = static_cast<CGFloat>(spec.startPoint.y + yOffset - panelSize * 0.5);
    [panel setFrame:NSMakeRect(x, y, panelSize, panelSize) display:NO];

    const double fontSize = spec.baseFontSize * scale;
    NSFont* font = ResolveLabelFont(spec, fontSize);
    const CGFloat clampedFontSize = static_cast<CGFloat>(std::max(6.0, fontSize));
    label.fontSize = clampedFontSize;
    if (font != nil) {
        NSString* fontName = [font fontName];
        if (fontName != nil) {
            label.font = (__bridge CFTypeRef)fontName;
        } else {
            label.font = (__bridge CFTypeRef)@"Helvetica-Bold";
        }
    } else {
        label.font = (__bridge CFTypeRef)@"Helvetica-Bold";
    }
    label.foregroundColor = [ColorFromArgb(spec.argb, alphaFactor) CGColor];
}

void StartTextAnimation(NSPanel* panel, CATextLayer* label, TextAnimationSpec spec) {
    if (panel == nil || label == nil) {
        return;
    }

    const uint64_t startTickMs = MonotonicNowMs();
    const uint64_t generation = AnimationGeneration().load(std::memory_order_acquire);

    auto step = std::make_shared<std::function<void()>>();
    *step = [panel, label, spec, startTickMs, generation, step]() {
        if (generation != AnimationGeneration().load(std::memory_order_acquire)) {
            CloseTrackedPanel(panel);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(ActivePanelsMutex());
            if (!IsPanelTrackedLocked(panel)) {
                return;
            }
        }

        const uint64_t nowMs = MonotonicNowMs();
        const uint64_t elapsedMs = (nowMs >= startTickMs) ? (nowMs - startTickMs) : 0;
        const double t = Clamp01(static_cast<double>(elapsedMs) / std::max(1.0, spec.durationMs));
        ApplyTextFrame(panel, label, spec, t);

        if (t >= 1.0) {
            CloseTrackedPanel(panel);
            return;
        }

        // Capture `step` (strong ref) to keep the animation chain alive.
        // The cycle breaks when t >= 1.0 or generation mismatch.
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
      std::vector<NSPanel*> toClose;
      {
          std::lock_guard<std::mutex> lock(ActivePanelsMutex());
          auto& panels = ActivePanels();
          toClose.reserve(panels.size());
          for (const auto& item : panels) {
              if (item.panel != nil) {
                  toClose.push_back(item.panel);
              }
          }
          panels.clear();
      }
      diagnostics::SetTextEffectFallbackActivePanels(0);
      for (NSPanel* panel : toClose) {
          [panel orderOut:nil];
          [panel release];
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

    NSLog(@"[MFX-TextFallback] ShowText called: pt=(%d,%d) color=0x%08X fontSize=%.1f",
          pt.x, pt.y, color.value, config.fontSize);

    diagnostics::RecordTextEffectFallbackShow(pt, text);

    const std::string utf8Text = Utf16ToUtf8(text.c_str());
    if (utf8Text.empty()) {
        diagnostics::RecordTextEffectFallbackError("utf8_empty");
        return;
    }
    NSString* nsText = [[NSString alloc] initWithUTF8String:utf8Text.c_str()];
    if (nsText == nil) {
        diagnostics::RecordTextEffectFallbackError("ns_text_nil");
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
    // Match Windows: fontSize(pt) -> px via 96/72 scaling.
    // No hardcoded minimum — let the user's configured size control rendering.
    const double baseFontPx = static_cast<double>(config.fontSize) * (96.0 / 72.0);
    spec.baseFontSize = std::max(baseFontPx, 6.0);
    spec.argb = color.value;
    spec.fontFamilyUtf8 = Utf16ToUtf8(config.fontFamily.c_str());
    spec.emojiText = settings::HasEmojiStarter(text);

    RunOnMainThreadAsync(^{
      NSString* textCopy = nsText;

      const double panelSize = ResolvePanelSize(spec.baseFontSize);

      NSPanel* panel = [[NSPanel alloc] initWithContentRect:NSMakeRect(0.0, 0.0, panelSize, panelSize)
                                                   styleMask:NSWindowStyleMaskBorderless
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
      if (panel == nil) {
          diagnostics::RecordTextEffectFallbackError("panel_nil");
          [textCopy release];
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
      content.layer.backgroundColor = [[NSColor clearColor] CGColor];

      // Vertically center text: place the label in the middle of the panel.
      // CATextLayer renders from the top of its frame, so offset the origin
      // to vertically center a single line of text.
      const CGFloat labelHeight = static_cast<CGFloat>(spec.baseFontSize * 2.0);
      const CGFloat labelY = (static_cast<CGFloat>(panelSize) - labelHeight) * 0.5;
      CATextLayer* label = [CATextLayer layer];
      label.frame = CGRectMake(0.0, labelY, static_cast<CGFloat>(panelSize), labelHeight);
      label.alignmentMode = kCAAlignmentCenter;
      label.wrapped = YES;
      label.truncationMode = kCATruncationEnd;
      label.contentsScale = std::max<CGFloat>(1.0, [panel backingScaleFactor]);
      label.string = textCopy;
      label.foregroundColor = [ColorFromArgb(spec.argb, 1.0) CGColor];
      [content.layer addSublayer:label];

      {
          std::lock_guard<std::mutex> lock(ActivePanelsMutex());
          ActivePanels().push_back(ActivePanel{panel});
          diagnostics::SetTextEffectFallbackActivePanels(ActivePanels().size());
      }
      EnforceWindowCap(maxConcurrentWindows_);

      [panel orderFrontRegardless];
      diagnostics::RecordTextEffectFallbackPanelCreated();
      StartTextAnimation(panel, label, spec);
      [textCopy release];
    });
#endif
}

} // namespace mousefx
