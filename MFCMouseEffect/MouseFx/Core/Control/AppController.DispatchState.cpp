#include "pch.h"

#include "AppController.h"

#include <algorithm>
#include <cmath>

#include "MouseFx/Core/Overlay/InputIndicatorKeyFilter.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#elif MFX_PLATFORM_WINDOWS
#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"
#endif

namespace mousefx {
namespace {

std::string ResolveInputIndicatorWasmRouteReason(
    const InputIndicatorWasmRouteTrace& trace,
    bool renderedByWasm,
    bool wasmFallbackEnabled,
    bool nativeFallbackApplied) {
    if (renderedByWasm) {
        return "wasm_rendered";
    }
    if (!wasmFallbackEnabled) {
        return "fallback_disabled";
    }
    if (!trace.anchorsResolved) {
        return "anchor_unavailable";
    }
    if (!trace.hostPresent || !trace.hostEnabled || !trace.pluginLoaded) {
        return "plugin_unloaded";
    }
    if (!trace.eventSupported) {
        return "event_not_supported";
    }
    if (trace.invokeAttempted) {
        return nativeFallbackApplied ? "invoke_failed_no_output" : "invoke_no_output";
    }
    return nativeFallbackApplied ? "invoke_failed_no_output" : "unknown";
}

} // namespace

bool AppController::ConsumeIgnoreNextClick() {
    if (!ignoreNextClick_) {
        return false;
    }
    ignoreNextClick_ = false;
    return true;
}

void AppController::OnGlobalKey(const KeyEvent& ev) {
    InputAutomation().OnKey(ev);
    if (!ev.keyDown) {
        return;
    }
    const bool captureActiveBefore = shortcutCaptureSession_.IsActive();
    shortcutCaptureSession_.OnKeyDown(ev);
    const bool captureActiveAfter = shortcutCaptureSession_.IsActive();
    hook_->SetKeyboardCaptureExclusive(captureActiveAfter);
    if (captureActiveBefore || captureActiveAfter) {
        return;
    }
    DispatchInputIndicatorKey(ev);
}

void AppController::DispatchInputIndicatorClick(const ClickEvent& ev) {
    const InputIndicatorConfig& cfg = config_.inputIndicator;
    if (!cfg.enabled) {
        return;
    }

    if (cfg.renderMode == "wasm") {
        InputIndicatorWasmRouteTrace trace{};
        bool renderedByWasm = false;
        inputIndicatorWasmDispatch_.RouteClick(*this, ev, &renderedByWasm, &trace);
        const bool nativeFallbackApplied = !renderedByWasm && cfg.wasmFallbackToNative;
        RecordInputIndicatorWasmRouteStatus(
            "click", trace, renderedByWasm, cfg.wasmFallbackToNative, nativeFallbackApplied);
        if (renderedByWasm || !cfg.wasmFallbackToNative) {
            return;
        }
    }
    inputIndicatorOverlay_->OnClick(ev);
}

void AppController::DispatchInputIndicatorScroll(const ScrollEvent& ev) {
    const InputIndicatorConfig& cfg = config_.inputIndicator;
    if (!cfg.enabled) {
        return;
    }

    if (cfg.renderMode == "wasm") {
        InputIndicatorWasmRouteTrace trace{};
        bool renderedByWasm = false;
        inputIndicatorWasmDispatch_.RouteScroll(*this, ev, &renderedByWasm, &trace);
        const bool nativeFallbackApplied = !renderedByWasm && cfg.wasmFallbackToNative;
        RecordInputIndicatorWasmRouteStatus(
            "scroll", trace, renderedByWasm, cfg.wasmFallbackToNative, nativeFallbackApplied);
        if (renderedByWasm || !cfg.wasmFallbackToNative) {
            return;
        }
    }
    inputIndicatorOverlay_->OnScroll(ev);
}

void AppController::DispatchInputIndicatorKey(const KeyEvent& ev) {
    const InputIndicatorConfig& cfg = config_.inputIndicator;
    if (!ShouldShowInputIndicatorKey(cfg, ev)) {
        return;
    }

    if (cfg.renderMode == "wasm") {
        InputIndicatorWasmRouteTrace trace{};
        bool renderedByWasm = false;
        inputIndicatorWasmDispatch_.RouteKey(*this, ev, &renderedByWasm, &trace);
        const bool nativeFallbackApplied = !renderedByWasm && cfg.wasmFallbackToNative;
        RecordInputIndicatorWasmRouteStatus(
            "key", trace, renderedByWasm, cfg.wasmFallbackToNative, nativeFallbackApplied);
        if (renderedByWasm || !cfg.wasmFallbackToNative) {
            return;
        }
    }
    inputIndicatorOverlay_->OnKey(ev);
}

AppController::InputIndicatorWasmRouteStatus AppController::ReadInputIndicatorWasmRouteStatus() const {
    std::lock_guard<std::mutex> guard(inputIndicatorWasmRouteStatusMutex_);
    return inputIndicatorWasmRouteStatus_;
}

void AppController::RecordInputIndicatorWasmRouteStatus(
    const char* eventKind,
    const InputIndicatorWasmRouteTrace& trace,
    bool renderedByWasm,
    bool wasmFallbackEnabled,
    bool nativeFallbackApplied) {
    InputIndicatorWasmRouteStatus next{};
    next.eventKind = eventKind ? eventKind : "";
    next.renderMode = config_.inputIndicator.renderMode;
    next.eventTickMs = CurrentTickMs();
    next.routeAttempted = trace.routeAttempted;
    next.anchorsResolved = trace.anchorsResolved;
    next.hostPresent = trace.hostPresent;
    next.hostEnabled = trace.hostEnabled;
    next.pluginLoaded = trace.pluginLoaded;
    next.eventSupported = trace.eventSupported;
    next.invokeAttempted = trace.invokeAttempted;
    next.renderedByWasm = renderedByWasm || trace.renderedAny;
    next.wasmFallbackEnabled = wasmFallbackEnabled;
    next.nativeFallbackApplied = nativeFallbackApplied;
    next.reason = ResolveInputIndicatorWasmRouteReason(
        trace,
        next.renderedByWasm,
        wasmFallbackEnabled,
        nativeFallbackApplied);

    std::lock_guard<std::mutex> guard(inputIndicatorWasmRouteStatusMutex_);
    inputIndicatorWasmRouteStatus_ = std::move(next);
}

std::string AppController::StartShortcutCaptureSession(uint64_t timeoutMs) {
    const std::string sessionId = shortcutCaptureSession_.Start(timeoutMs);
    hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
    return sessionId;
}

void AppController::StopShortcutCaptureSession(const std::string& sessionId) {
    shortcutCaptureSession_.Stop(sessionId);
    hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
}

ShortcutCaptureSession::PollResult AppController::PollShortcutCaptureSession(const std::string& sessionId) {
    ShortcutCaptureSession::PollResult result = shortcutCaptureSession_.Poll(sessionId);
    hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
    return result;
}

bool AppController::ConsumeLatestMove(ScreenPoint* outPt) {
    if (!outPt) {
        return false;
    }
    return hook_->ConsumeLatestMove(*outPt);
}

uint64_t AppController::CurrentTickMs() const {
    if (!monotonicClockService_) {
        return 0;
    }
    return monotonicClockService_->NowMs();
}

uint32_t AppController::CurrentHoldDurationMs() const {
    if (!holdButtonDown_ || holdDownTick_ == 0) {
        return 0;
    }

    const uint64_t now = CurrentTickMs();
    const uint64_t delta = (now >= holdDownTick_) ? (now - holdDownTick_) : 0;
    return static_cast<uint32_t>(std::min<uint64_t>(delta, 0xFFFFFFFFu));
}

void AppController::BeginHoldTracking(const ScreenPoint& pt, int button) {
    holdButtonDown_ = true;
    holdTrackingButton_ = button;
    holdDownTick_ = CurrentTickMs();
    pendingHold_.pt = pt;
    pendingHold_.button = button;
    pendingHold_.active = true;
    ignoreNextClick_ = false;
}

void AppController::EndHoldTracking() {
    holdButtonDown_ = false;
    holdTrackingButton_ = 0;
    holdDownTick_ = 0;
}

void AppController::ArmHoldTimer() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->SetTimer(kHoldTimerId, kHoldDelayMs);
    }
}

void AppController::DisarmHoldTimer() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoldTimerId);
    }
}

void AppController::ArmHoldUpdateTimer() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->SetTimer(kHoldUpdateTimerId, kHoldUpdateIntervalMs);
    }
}

void AppController::DisarmHoldUpdateTimer() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoldUpdateTimerId);
    }
}

void AppController::ArmWasmFrameTimer() {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) {
        return;
    }
    dispatchMessageHost_->SetTimer(kWasmFrameTimerId, ResolveWasmFrameTimerIntervalMs());
}

void AppController::DisarmWasmFrameTimer() {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) {
        return;
    }
    dispatchMessageHost_->KillTimer(kWasmFrameTimerId);
}

void AppController::ClearPendingHold() {
    pendingHold_.active = false;
}

void AppController::CancelPendingHold() {
    if (!pendingHold_.active) {
        return;
    }
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoldTimerId);
    }
    pendingHold_.active = false;
}

bool AppController::ConsumePendingHold(ScreenPoint* outPt, int* outButton) {
    if (!pendingHold_.active) {
        return false;
    }
    if (outPt) {
        *outPt = pendingHold_.pt;
    }
    if (outButton) {
        *outButton = pendingHold_.button;
    }
    pendingHold_.active = false;
    return true;
}

void AppController::MarkIgnoreNextClick() {
    ignoreNextClick_ = true;
}

bool AppController::TryEnterHover(ScreenPoint* outPt) {
    if (hovering_) {
        return false;
    }

    const uint64_t elapsed = CurrentTickMs() - lastInputTime_;
    if (elapsed < kHoverThresholdMs) {
        return false;
    }

    hovering_ = true;
    if (outPt) {
        if (QueryCursorScreenPoint(outPt)) {
            RememberLastPointerPoint(*outPt);
        } else if (!TryGetLastPointerPoint(outPt)) {
            outPt->x = 0;
            outPt->y = 0;
        }
    }
    return true;
}

bool AppController::QueryCursorScreenPoint(ScreenPoint* outPt) const {
    if (!outPt || !cursorPositionService_) {
        return false;
    }
    return cursorPositionService_->TryGetCursorScreenPoint(outPt);
}

void AppController::RememberLastPointerPoint(const ScreenPoint& pt) {
    lastPointerPoint_ = pt;
    hasLastPointerPoint_ = true;
}

bool AppController::TryGetLastPointerPoint(ScreenPoint* outPt) const {
    if (!outPt || !hasLastPointerPoint_) {
        return false;
    }
    *outPt = lastPointerPoint_;
    return true;
}

uint32_t AppController::ResolveWasmFrameTimerIntervalMs() const {
    ScreenPoint pt{};
    if (!QueryCursorScreenPoint(&pt) && !TryGetLastPointerPoint(&pt)) {
        pt.x = 0;
        pt.y = 0;
    }

    int intervalMs = 16;
#if MFX_PLATFORM_MACOS
    intervalMs = macos_overlay_support::ResolveOverlayTimerIntervalMs(pt);
#elif MFX_PLATFORM_WINDOWS
    intervalMs = win32_overlay_timer_support::ResolveTimerIntervalMsForScreenPoint(pt.x, pt.y);
#else
    const int targetFps = (config_.overlayTargetFps > 0) ? config_.overlayTargetFps : 60;
    intervalMs = static_cast<int>(
        std::lround(1000.0 / static_cast<double>(std::max(1, targetFps))));
#endif
    return static_cast<uint32_t>(std::clamp(intervalMs, 4, 1000));
}

std::string AppController::CurrentForegroundProcessBaseName() {
    if (!foregroundProcessService_) {
        return {};
    }
    return foregroundProcessService_->CurrentProcessBaseName();
}

bool AppController::InjectShortcutForTest(const std::string& chordText) {
    if (!keyboardInjector_) {
        return false;
    }
    return keyboardInjector_->SendChord(chordText);
}

void AppController::KillDispatchTimer(uintptr_t timerId) {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) {
        return;
    }
    dispatchMessageHost_->KillTimer(timerId);
}

} // namespace mousefx
