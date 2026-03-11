#include "pch.h"

#include "Platform/macos/Effects/MacosTextEffectFallback.h"

#include "MouseFx/Core/Diagnostics/TextEffectRuntimeDiagnostics.h"
#include "MouseFx/Core/Effects/TextEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTextEffectFallbackSwiftBridge.h"
#include "Settings/EmojiUtils.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <utility>
#include <vector>

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#endif

namespace mousefx {

#if defined(__APPLE__)
namespace {

struct ActivePanel final {
    void* handle = nullptr;
};

struct TextAnimationCommand final {
    ScreenPoint startPoint{};
    TextEffectRenderCommand command{};
    std::string fontFamilyUtf8{};
};

struct TextAnimationState final {
    void* panelHandle = nullptr;
    TextAnimationCommand cmd{};
    uint64_t startTickMs = 0;
    uint64_t generation = 0;
    int timerIntervalMs = 16;
};

struct ShowTextContext final {
    std::string utf8Text{};
    TextAnimationCommand cmd{};
    size_t cap = 1;
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

void ApplyTextFrame(void* panelHandle, const TextAnimationCommand& cmd, double t) {
    if (panelHandle == nullptr) {
        return;
    }

    const TextEffectRenderFrame frame = ComputeTextEffectRenderFrame(cmd.command, t);
    const double panelSize = std::max(1.0, cmd.command.panelSizePx);
    const double x = static_cast<double>(cmd.startPoint.x) + frame.offsetXPx - panelSize * 0.5;
    const double y = static_cast<double>(cmd.startPoint.y) + frame.offsetYUpPx - panelSize * 0.5;
    mfx_macos_text_panel_set_frame_v1(panelHandle, x, y, panelSize);

    const char* fontFamilyUtf8 = cmd.fontFamilyUtf8.empty() ? "" : cmd.fontFamilyUtf8.c_str();
    mfx_macos_text_panel_apply_style_v1(
        panelHandle,
        frame.fontSizePx,
        cmd.command.argb,
        frame.alpha,
        fontFamilyUtf8,
        cmd.command.emojiText ? 1 : 0);
}

void TickTextAnimation(void* opaque);

void ScheduleTextAnimationTick(TextAnimationState* state) {
    if (state == nullptr) {
        return;
    }
    const int delayMs = std::clamp(state->timerIntervalMs, 4, 1000);
    dispatch_after_f(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(delayMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        state,
        &TickTextAnimation);
}

void TickTextAnimation(void* opaque) {
    std::unique_ptr<TextAnimationState> state(static_cast<TextAnimationState*>(opaque));
    if (!state || state->panelHandle == nullptr) {
        return;
    }

    if (state->generation != AnimationGeneration().load(std::memory_order_acquire)) {
        CloseTrackedPanel(state->panelHandle);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(ActivePanelsMutex());
        if (!IsPanelTrackedLocked(state->panelHandle)) {
            return;
        }
    }

    const uint64_t nowMs = MonotonicNowMs();
    const uint64_t elapsedMs = (nowMs >= state->startTickMs) ? (nowMs - state->startTickMs) : 0;
    const double durationMs = std::max(1, state->cmd.command.durationMs);
    const double t = std::clamp(static_cast<double>(elapsedMs) / static_cast<double>(durationMs), 0.0, 1.0);
    ApplyTextFrame(state->panelHandle, state->cmd, t);

    if (t >= 1.0) {
        CloseTrackedPanel(state->panelHandle);
        return;
    }

    ScheduleTextAnimationTick(state.release());
}

void StartTextAnimation(void* panelHandle, TextAnimationCommand cmd) {
    if (panelHandle == nullptr) {
        return;
    }
    const int timerIntervalMs = macos_overlay_support::ResolveOverlayTimerIntervalMs(cmd.startPoint);
    auto* state = new TextAnimationState{
        panelHandle,
        std::move(cmd),
        MonotonicNowMs(),
        AnimationGeneration().load(std::memory_order_acquire),
        timerIntervalMs,
    };
    TickTextAnimation(state);
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
    macos_overlay_support::RunOnMainThreadSync(
        [](void*) {
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
        },
        nullptr);
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
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> driftXDist(-50, 49);
    std::uniform_int_distribution<int> swayFreqDist(0, 199);
    std::uniform_int_distribution<int> swayAmpDist(0, 99);

    TextEffectRandomSamples randomSamples{};
    if (config.floatDistance <= 0) {
        randomSamples.driftX = 0;
        randomSamples.swayFreqCenti = 0;
        randomSamples.swayAmpDeci = 0;
    } else {
        randomSamples.driftX = driftXDist(rng);
        randomSamples.swayFreqCenti = swayFreqDist(rng);
        randomSamples.swayAmpDeci = swayAmpDist(rng);
    }

    const TextEffectRenderCommand command = ComputeTextEffectRenderCommand(
        text,
        color,
        config,
        settings::HasEmojiStarter(text),
        randomSamples);
    ShowTextComputed(pt, command);
#endif
}

void MacosTextEffectFallback::ShowTextComputed(
    const ScreenPoint& anchorPoint,
    const TextEffectRenderCommand& command) {
#if !defined(__APPLE__)
    (void)anchorPoint;
    (void)command;
    return;
#else
    if (command.text.empty()) {
        diagnostics::RecordTextEffectFallbackError("empty_text");
        return;
    }
    if (maxConcurrentWindows_ == 0) {
        maxConcurrentWindows_ = 8;
    }

    diagnostics::RecordTextEffectFallbackShow(anchorPoint, command.text);

    const std::string utf8Text = Utf16ToUtf8(command.text.c_str());
    if (utf8Text.empty()) {
        diagnostics::RecordTextEffectFallbackError("utf8_empty");
        return;
    }

    TextAnimationCommand animCmd{};
    animCmd.startPoint = ScreenToOverlayPoint(anchorPoint);
    animCmd.command = command;
    animCmd.fontFamilyUtf8 = Utf16ToUtf8(command.fontFamily.c_str());
    auto* context = new ShowTextContext{
        utf8Text,
        animCmd,
        maxConcurrentWindows_,
    };
    macos_overlay_support::RunOnMainThreadAsync(
        [](void* opaque) {
            std::unique_ptr<ShowTextContext> context(
                static_cast<ShowTextContext*>(opaque));
            if (!context) {
                return;
            }
            const double panelSize = std::max(1.0, context->cmd.command.panelSizePx);
            const char* fontFamilyUtf8 = context->cmd.fontFamilyUtf8.empty()
                ? ""
                : context->cmd.fontFamilyUtf8.c_str();
            void* panelHandle = mfx_macos_text_panel_create_v1(
                context->utf8Text.c_str(),
                panelSize,
                context->cmd.command.baseFontSizePx,
                context->cmd.command.argb,
                fontFamilyUtf8,
                context->cmd.command.emojiText ? 1 : 0);
            if (panelHandle == nullptr) {
                diagnostics::RecordTextEffectFallbackError("panel_nil");
                return;
            }

            {
                std::lock_guard<std::mutex> lock(ActivePanelsMutex());
                ActivePanels().push_back(ActivePanel{panelHandle});
                diagnostics::SetTextEffectFallbackActivePanels(ActivePanels().size());
            }
            EnforceWindowCap(context->cap);

            mfx_macos_text_panel_show_v1(panelHandle);
            diagnostics::RecordTextEffectFallbackPanelCreated();
            StartTextAnimation(panelHandle, std::move(context->cmd));
        },
        context);
#endif
}

} // namespace mousefx
