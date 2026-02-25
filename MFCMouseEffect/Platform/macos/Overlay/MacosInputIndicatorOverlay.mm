#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>
#include <utility>

namespace mousefx {

namespace {

int ClampInt(int value, int lo, int hi) {
    return std::max(lo, std::min(value, hi));
}

std::string MouseButtonLabel(MouseButton button) {
    switch (button) {
    case MouseButton::Left:
        return "L";
    case MouseButton::Right:
        return "R";
    case MouseButton::Middle:
        return "M";
    default:
        return "?";
    }
}

std::string ScrollLabel(int delta) {
    return delta >= 0 ? "SCR +" : "SCR -";
}

std::string KeyLabel(const KeyEvent& ev) {
    if (!ev.text.empty()) {
        return Utf16ToUtf8(ev.text.c_str());
    }
    std::ostringstream oss;
    if (ev.ctrl) {
        oss << "Ctrl+";
    }
    if (ev.shift) {
        oss << "Shift+";
    }
    if (ev.alt) {
        oss << "Alt+";
    }
    if (ev.meta || ev.win) {
        oss << "Cmd+";
    }
    oss << "K" << ev.vkCode;
    return oss.str();
}

#if defined(__APPLE__)
void RunOnMainThreadSync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    if ([NSThread isMainThread]) {
        block();
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), block);
}

void RunOnMainThreadAsync(dispatch_block_t block) {
    if (!block) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), block);
}

void FlushMainThreadQueueSync() {
    if ([NSThread isMainThread]) {
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), ^{});
}

NSString* NsStringFromUtf8(const std::string& text) {
    if (text.empty()) {
        return @"";
    }
    NSString* ns = [NSString stringWithUTF8String:text.c_str()];
    return ns ? ns : @"";
}
#endif

} // namespace

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

    RunOnMainThreadSync(^{
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

    RunOnMainThreadSync(^{
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
    RunOnMainThreadAsync(^{
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

void MacosInputIndicatorOverlay::OnClick(const ClickEvent& ev) {
    ShowAt(ev.pt, MouseButtonLabel(ev.button));
}

void MacosInputIndicatorOverlay::OnScroll(const ScrollEvent& ev) {
    ShowAt(ev.pt, ScrollLabel(ev.delta));
}

void MacosInputIndicatorOverlay::OnKey(const KeyEvent& ev) {
    if (!ShouldShowKeyboard()) {
        return;
    }
    ShowAt(ev.pt, KeyLabel(ev));
}

bool MacosInputIndicatorOverlay::ReadDebugState(InputIndicatorDebugState* outState) const {
    if (!outState) {
        return false;
    }
    std::lock_guard<std::mutex> lock(debugMutex_);
    outState->lastAppliedLabel = lastAppliedLabel_;
    outState->applyCount = applyCount_;
    return true;
}

bool MacosInputIndicatorOverlay::RunMouseLabelProbe(std::vector<std::string>* outAppliedLabels) {
    if (outAppliedLabels) {
        outAppliedLabels->clear();
    }

    InputIndicatorConfig oldConfig{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return false;
        }
        oldConfig = config_;
        config_.enabled = true;
        config_.durationMs = std::max(120, oldConfig.durationMs);
    }

    const auto applyLabelAndCapture = [this, outAppliedLabels](const std::string& label) {
        ShowAt(ScreenPoint{128, 128}, label);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
#if defined(__APPLE__)
        FlushMainThreadQueueSync();
#endif
        InputIndicatorDebugState state{};
        if (!ReadDebugState(&state)) {
            return false;
        }
        if (outAppliedLabels) {
            outAppliedLabels->push_back(state.lastAppliedLabel);
        }
        return state.lastAppliedLabel == label;
    };

    const bool leftOk = applyLabelAndCapture("L");
    const bool rightOk = applyLabelAndCapture("R");
    const bool middleOk = applyLabelAndCapture("M");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = oldConfig;
    }

    return leftOk && rightOk && middleOk;
}

bool MacosInputIndicatorOverlay::RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels) {
    if (outAppliedLabels) {
        outAppliedLabels->clear();
    }

    InputIndicatorConfig oldConfig{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return false;
        }
        oldConfig = config_;
        config_.enabled = true;
        config_.keyboardEnabled = true;
        config_.durationMs = std::max(120, oldConfig.durationMs);
    }

    const auto applyKeyAndCapture =
        [this, outAppliedLabels](const KeyEvent& ev, const std::string& expectedLabel) {
            OnKey(ev);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
#if defined(__APPLE__)
            FlushMainThreadQueueSync();
#endif
            InputIndicatorDebugState state{};
            if (!ReadDebugState(&state)) {
                return false;
            }
            if (outAppliedLabels) {
                outAppliedLabels->push_back(state.lastAppliedLabel);
            }
            return state.lastAppliedLabel == expectedLabel;
        };

    KeyEvent textKey{};
    textKey.pt = ScreenPoint{128, 128};
    textKey.text = L"A";
    const bool textOk = applyKeyAndCapture(textKey, "A");

    KeyEvent metaKey{};
    metaKey.pt = ScreenPoint{128, 128};
    metaKey.meta = true;
    metaKey.vkCode = 9;
    const bool metaOk = applyKeyAndCapture(metaKey, "Cmd+K9");

    KeyEvent plainKey{};
    plainKey.pt = ScreenPoint{128, 128};
    plainKey.vkCode = 6;
    const bool plainOk = applyKeyAndCapture(plainKey, "K6");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = oldConfig;
    }

    return textOk && metaOk && plainOk;
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

    const int sizePx = ClampInt(cfg.sizePx, 28, 220);
    const int durationMs = ClampInt(cfg.durationMs, 80, 5000);
    const bool absolute = (ToLowerAscii(TrimAscii(cfg.positionMode)) == "absolute");
    const ScreenPoint overlayPt = absolute ? pt : ScreenToOverlayPoint(pt);
    const int x = absolute ? cfg.absoluteX : (overlayPt.x + cfg.offsetX);
    const int y = absolute ? cfg.absoluteY : (overlayPt.y + cfg.offsetY);
    const uint64_t generation = displayGeneration_.fetch_add(1, std::memory_order_acq_rel) + 1;
    const std::string labelCopy = label;

#if defined(__APPLE__)
    RunOnMainThreadAsync(^{
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
      [text setStringValue:NsStringFromUtf8(labelCopy)];
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
