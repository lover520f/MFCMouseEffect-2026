#include "pch.h"

#include "AppController.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

#include "MouseFx/Core/Overlay/InputIndicatorKeyFilter.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Pet/MacosMouseCompanionPhase1SwiftBridge.h"
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

MouseCompanionConfig ResolveActiveMouseCompanionConfig(const MouseCompanionConfig& config) {
    MouseCompanionConfig active = config;
    if (active.useTestProfile) {
        active.pressLiftPx = active.testPressLiftPx;
        active.smoothingPercent = active.testSmoothingPercent;
        active.clickStreakBreakMs = active.testClickStreakBreakMs;
        active.headTintPerClick = active.testHeadTintPerClick;
        active.headTintMax = active.testHeadTintMax;
        active.headTintDecayPerSecond = active.testHeadTintDecayPerSecond;
    }
    return active;
}

constexpr int kPetActionIdle = 0;
constexpr int kPetActionFollow = 1;
constexpr int kPetActionClickReact = 2;
constexpr int kPetActionDrag = 3;
constexpr int kPetActionHoldReact = 4;
constexpr int kPetActionScrollReact = 5;
constexpr int32_t kPetPoseBoneCount = 6;
constexpr int32_t kPetBoneLeftEar = 0;
constexpr int32_t kPetBoneRightEar = 1;
constexpr int32_t kPetBoneLeftHand = 2;
constexpr int32_t kPetBoneRightHand = 3;
constexpr int32_t kPetBoneLeftLeg = 4;
constexpr int32_t kPetBoneRightLeg = 5;
constexpr std::array<const char*, kPetPoseBoneCount> kPetPoseBoneNames = {
    "left_ear",
    "right_ear",
    "left_hand",
    "right_hand",
    "left_leg",
    "right_leg",
};

struct PetVisualMotionProfile {
    float clickActionPulseBase;
    float clickActionPulseStreakStep;
    float clickActionPulseMax;
    float holdDecayPerSecond;
    float scrollDecayPerSecond;
    float holdPulseFloor;
    float scrollPulseFloor;
    float holdPoseGain;
    float scrollPoseGain;
    float followPoseGain;
    float followWalkRateMinHz;
    float followWalkRateMaxHz;
};

// Production profile: tuned for tauri-like click burst with stable daily behavior.
constexpr PetVisualMotionProfile kPetVisualMotionProd = {
    0.84f, // clickActionPulseBase
    0.05f, // clickActionPulseStreakStep
    1.0f,  // clickActionPulseMax
    1.2f,  // holdDecayPerSecond
    1.8f,  // scrollDecayPerSecond
    0.35f, // holdPulseFloor
    0.30f, // scrollPulseFloor
    1.18f, // holdPoseGain
    1.0f,  // scrollPoseGain
    0.62f, // followPoseGain
    1.45f, // followWalkRateMinHz
    2.60f, // followWalkRateMaxHz
};

// Test profile: faster and stronger feedback to reduce manual verification cycle time.
constexpr PetVisualMotionProfile kPetVisualMotionTest = {
    0.92f, // clickActionPulseBase
    0.06f, // clickActionPulseStreakStep
    1.0f,  // clickActionPulseMax
    1.6f,  // holdDecayPerSecond
    2.4f,  // scrollDecayPerSecond
    0.45f, // holdPulseFloor
    0.40f, // scrollPulseFloor
    1.30f, // holdPoseGain
    1.10f, // scrollPoseGain
    0.74f, // followPoseGain
    1.75f, // followWalkRateMinHz
    3.00f, // followWalkRateMaxHz
};

const PetVisualMotionProfile& ResolvePetVisualMotionProfile(bool useTestProfile) {
    return useTestProfile ? kPetVisualMotionTest : kPetVisualMotionProd;
}

constexpr double kPetFollowSpeedThresholdPxPerSec = 45.0;
constexpr double kPetFollowSpeedThresholdTestPxPerSec = 30.0;
constexpr double kPetPointerSpeedDecayPerSecond = 8.5;
constexpr double kPetPointerSpeedDecayTestPerSecond = 6.0;
constexpr double kPetPointerSpeedZeroFloorPxPerSec = 0.5;
constexpr float kPetHoverIdleIntensity = 0.42f;
constexpr float kPetHoverIdleIntensityTest = 0.58f;
constexpr float kPetScrollAmplitudeMin = 0.35f;
constexpr float kPetScrollAmplitudeMax = 2.4f;
constexpr uint32_t kPetScrollQueueMax = 8;
constexpr uint32_t kPetScrollFastIntervalMs = 24;
constexpr uint32_t kPetScrollSlowIntervalMs = 180;

float ResolvePetScrollMotionDurationMaxSec(bool useTestProfile) {
    return useTestProfile ? 0.20f : 0.26f;
}

float ResolvePetScrollMotionDurationMinSec(bool useTestProfile) {
    return useTestProfile ? 0.08f : 0.10f;
}

double ResolvePetFollowSpeedThresholdPxPerSec(bool useTestProfile) {
    return useTestProfile ? kPetFollowSpeedThresholdTestPxPerSec : kPetFollowSpeedThresholdPxPerSec;
}

double ResolvePetPointerSpeedDecayPerSecond(bool useTestProfile) {
    return useTestProfile ? kPetPointerSpeedDecayTestPerSecond : kPetPointerSpeedDecayPerSecond;
}

float ResolvePetHoverIdleIntensity(bool useTestProfile) {
    return useTestProfile ? kPetHoverIdleIntensityTest : kPetHoverIdleIntensity;
}

float ClampUnit(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float ImpulseProfile(float t, float inRatio, float holdRatio) {
    const float clampedT = ClampUnit(t);
    const float inPart = std::clamp(inRatio, 0.05f, 0.9f);
    const float holdPart = std::clamp(holdRatio, 0.0f, 0.7f);
    const float outStart = std::clamp(inPart + holdPart, inPart, 0.98f);
    if (clampedT < inPart) {
        const float p = clampedT / inPart;
        return 1.0f - std::pow(1.0f - p, 3.0f);
    }
    if (clampedT < outStart) {
        return 1.0f;
    }
    const float p = (clampedT - outStart) / std::max(0.001f, 1.0f - outStart);
    return 1.0f - p * p * (3.0f - 2.0f * p);
}

void WriteQuaternionFromEuler(float rx, float ry, float rz, float* out) {
    if (!out) {
        return;
    }
    const float hx = rx * 0.5f;
    const float hy = ry * 0.5f;
    const float hz = rz * 0.5f;
    const float cx = std::cos(hx);
    const float sx = std::sin(hx);
    const float cy = std::cos(hy);
    const float sy = std::sin(hy);
    const float cz = std::cos(hz);
    const float sz = std::sin(hz);
    out[0] = sx * cy * cz - cx * sy * sz; // x
    out[1] = cx * sy * cz + sx * cy * sz; // y
    out[2] = cx * cy * sz - sx * sy * cz; // z
    out[3] = cx * cy * cz + sx * sy * sz; // w
}

double Distance(const ScreenPoint& a, const ScreenPoint& b) {
    const double dx = static_cast<double>(a.x - b.x);
    const double dy = static_cast<double>(a.y - b.y);
    return std::sqrt(dx * dx + dy * dy);
}

const char* ResolvePetActionName(int actionCode) {
    switch (actionCode) {
    case kPetActionFollow:
        return "follow";
    case kPetActionClickReact:
        return "click_react";
    case kPetActionDrag:
        return "drag";
    case kPetActionHoldReact:
        return "hold_react";
    case kPetActionScrollReact:
        return "scroll_react";
    case kPetActionIdle:
    default:
        return "idle";
    }
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

AppController::MouseCompanionRuntimeStatus AppController::ReadMouseCompanionRuntimeStatus() const {
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    return mouseCompanionRuntimeStatus_;
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

uint32_t AppController::ActiveHoverThresholdMs() const {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    return activeConfig.useTestProfile ? kHoverThresholdTestMs : kHoverThresholdMs;
}

uint32_t AppController::ActiveHoldDelayMs() const {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    return activeConfig.useTestProfile ? kHoldDelayTestMs : kHoldDelayMs;
}

uint32_t AppController::ResolvePetVisualHoldEnterMs(const MouseCompanionConfig& activeConfig) const {
    return activeConfig.useTestProfile ? kPetVisualHoldEnterTestMs : kPetVisualHoldEnterMs;
}

double AppController::ResolvePetVisualHoldStableSpeedThresholdPxPerSec(
    const MouseCompanionConfig& activeConfig) const {
    return activeConfig.useTestProfile
        ? kPetVisualHoldStableSpeedThresholdTestPxPerSec
        : kPetVisualHoldStableSpeedThresholdPxPerSec;
}

bool AppController::IsPetVisualHoldSuppressedByScroll(
    uint64_t nowTickMs,
    const MouseCompanionConfig& /*activeConfig*/) const {
    if (petLastScrollTickMs_ == 0 || nowTickMs < petLastScrollTickMs_) {
        return false;
    }
    return (nowTickMs - petLastScrollTickMs_) <= kPetVisualHoldSuppressAfterScrollMs;
}

bool AppController::ResolvePetVisualHoldState(
    const MouseCompanionConfig& activeConfig,
    uint64_t nowTickMs,
    float* outIntensity) const {
    if (outIntensity) {
        *outIntensity = 0.0f;
    }
    if (IsPetVisualHoldSuppressedByScroll(nowTickMs, activeConfig)) {
        return false;
    }

    const bool inReleaseBuffer =
        (petReleaseHoldUntilTickMs_ > 0) &&
        (nowTickMs < petReleaseHoldUntilTickMs_);
    bool enteredByPress = false;
    if (holdButtonDown_ &&
        petPrimaryPress_.active &&
        petPrimaryPress_.downTickMs > 0 &&
        nowTickMs >= petPrimaryPress_.downTickMs) {
        const uint64_t heldMs = nowTickMs - petPrimaryPress_.downTickMs;
        const bool stablePress =
            petPointerMotion_.moveSpeedPxPerSec <=
            ResolvePetVisualHoldStableSpeedThresholdPxPerSec(activeConfig);
        enteredByPress = stablePress && heldMs >= ResolvePetVisualHoldEnterMs(activeConfig);
    }

    if (!enteredByPress && !inReleaseBuffer) {
        return false;
    }

    float intensity = 1.0f;
    if (enteredByPress) {
        const float holdMs = static_cast<float>(CurrentHoldDurationMs());
        intensity = std::max(0.35f, ClampUnit(holdMs / 300.0f));
    }
    if (outIntensity) {
        *outIntensity = intensity;
    }
    return true;
}

void AppController::UpdatePetPrimaryPressTravel(const ScreenPoint& pt) {
    if (!petPrimaryPress_.active) {
        return;
    }
    const double travel = Distance(petPrimaryPress_.downPoint, pt);
    if (travel > petPrimaryPress_.maxTravelPx) {
        petPrimaryPress_.maxTravelPx = travel;
    }
}

bool AppController::EvaluatePetPrimaryClickEligibility(uint64_t nowTickMs) const {
    if (petLastScrollTickMs_ > 0 && nowTickMs >= petLastScrollTickMs_) {
        const uint64_t deltaMs = nowTickMs - petLastScrollTickMs_;
        if (deltaMs <= kPetClickSuppressAfterScrollMs) {
            return false;
        }
    }

    if (!petPrimaryPress_.releaseReady) {
        return false;
    }

    const uint64_t ageMs =
        (nowTickMs >= petPrimaryPress_.releaseTickMs)
            ? (nowTickMs - petPrimaryPress_.releaseTickMs)
            : 0;
    if (ageMs > 800) {
        return false;
    }
    if (petPrimaryPress_.holdTriggered) {
        return false;
    }
    if (petPrimaryPress_.releasePressMs > kPetClickMaxPressMs) {
        return false;
    }
    if (petPrimaryPress_.releaseMaxTravelPx > kPetClickMaxTravelPx) {
        return false;
    }
    return true;
}

void AppController::SyncPetClickStreakRuntimeStatus(const MouseCompanionConfig& activeConfig) {
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.clickStreak = petClickStreak_.streak;
    mouseCompanionRuntimeStatus_.clickStreakTintAmount = petClickStreak_.tintAmount;
    mouseCompanionRuntimeStatus_.clickStreakBreakMs = activeConfig.clickStreakBreakMs;
    mouseCompanionRuntimeStatus_.clickStreakDecayPerSecond =
        static_cast<float>(activeConfig.headTintDecayPerSecond);
}

void AppController::UpdatePetClickStreakDecay(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig) {
    if (petClickStreak_.lastUpdateTickMs == 0) {
        petClickStreak_.lastUpdateTickMs = nowTickMs;
        SyncPetClickStreakRuntimeStatus(activeConfig);
        return;
    }

    const uint64_t deltaMs =
        (nowTickMs >= petClickStreak_.lastUpdateTickMs)
            ? (nowTickMs - petClickStreak_.lastUpdateTickMs)
            : 0;
    petClickStreak_.lastUpdateTickMs = nowTickMs;

    if (petClickStreak_.streak > 0 &&
        petClickStreak_.lastClickTickMs > 0 &&
        nowTickMs >= petClickStreak_.lastClickTickMs) {
        const uint64_t sinceClickMs = nowTickMs - petClickStreak_.lastClickTickMs;
        if (sinceClickMs > static_cast<uint64_t>(std::max(0, activeConfig.clickStreakBreakMs))) {
            petClickStreak_.streak = 0;
            petClickStreak_.lastClickTickMs = 0;
        }
    }

    if (petClickStreak_.streak == 0 && petClickStreak_.tintAmount > 0.0f && deltaMs > 0) {
        const float dtSeconds = static_cast<float>(deltaMs) / 1000.0f;
        const float decay = static_cast<float>(activeConfig.headTintDecayPerSecond) * dtSeconds;
        petClickStreak_.tintAmount = std::max(0.0f, petClickStreak_.tintAmount - decay);
        if (petClickStreak_.tintAmount < 0.0001f) {
            petClickStreak_.tintAmount = 0.0f;
        }
    }

    SyncPetClickStreakRuntimeStatus(activeConfig);
}

void AppController::RegisterPetClickStreakClick(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig) {
    if (petClickStreak_.lastClickTickMs > 0 && nowTickMs >= petClickStreak_.lastClickTickMs) {
        const uint64_t sinceClickMs = nowTickMs - petClickStreak_.lastClickTickMs;
        if (sinceClickMs > static_cast<uint64_t>(std::max(0, activeConfig.clickStreakBreakMs))) {
            petClickStreak_.streak = 0;
        }
    } else if (petClickStreak_.lastClickTickMs == 0) {
        petClickStreak_.streak = 0;
    }

    petClickStreak_.streak = std::max(0, petClickStreak_.streak) + 1;
    petClickStreak_.lastClickTickMs = nowTickMs;
    petClickStreak_.lastUpdateTickMs = nowTickMs;
    const float tintGain = static_cast<float>(activeConfig.headTintPerClick);
    const float tintCap = static_cast<float>(activeConfig.headTintMax);
    petClickStreak_.tintAmount = std::min(tintCap, petClickStreak_.tintAmount + tintGain);
    SyncPetClickStreakRuntimeStatus(activeConfig);
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
        dispatchMessageHost_->SetTimer(kHoldTimerId, ActiveHoldDelayMs());
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
    if (elapsed < ActiveHoverThresholdMs()) {
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

void AppController::UpdatePetPointerMotion(const ScreenPoint& pt, uint64_t nowTickMs) {
    if (petPointerMotion_.hasSample && nowTickMs > petPointerMotion_.lastSampleTickMs) {
        const double dtSeconds =
            static_cast<double>(nowTickMs - petPointerMotion_.lastSampleTickMs) / 1000.0;
        if (dtSeconds > 0.0001) {
            petPointerMotion_.moveSpeedPxPerSec =
                Distance(pt, petPointerMotion_.lastSamplePoint) / dtSeconds;
        }
    }
    petPointerMotion_.hasSample = true;
    petPointerMotion_.lastSamplePoint = pt;
    petPointerMotion_.lastSampleTickMs = nowTickMs;
    petPointerMotion_.lastEvalTickMs = nowTickMs;
}

void AppController::DecayPetPointerMotion(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig) {
    if (!petPointerMotion_.hasSample || nowTickMs <= petPointerMotion_.lastEvalTickMs) {
        return;
    }
    const double dtSeconds =
        static_cast<double>(nowTickMs - petPointerMotion_.lastEvalTickMs) / 1000.0;
    petPointerMotion_.lastEvalTickMs = nowTickMs;
    const double pointerDecay =
        std::exp(-ResolvePetPointerSpeedDecayPerSecond(activeConfig.useTestProfile) * dtSeconds);
    petPointerMotion_.moveSpeedPxPerSec *= pointerDecay;
    if (petPointerMotion_.moveSpeedPxPerSec < kPetPointerSpeedZeroFloorPxPerSec) {
        petPointerMotion_.moveSpeedPxPerSec = 0.0;
    }
}

void AppController::ResolvePetContinuousAction(
    const MouseCompanionConfig& activeConfig,
    int* outActionCode,
    float* outActionIntensity) const {
    int actionCode = kPetActionIdle;
    float actionIntensity = hovering_ ? ResolvePetHoverIdleIntensity(activeConfig.useTestProfile) : 0.0f;
    const double threshold = ResolvePetFollowSpeedThresholdPxPerSec(activeConfig.useTestProfile);
    if (petPointerMotion_.moveSpeedPxPerSec > threshold) {
        actionCode = kPetActionFollow;
        const double speedAboveThreshold = petPointerMotion_.moveSpeedPxPerSec - threshold;
        const double speedWindow = std::max(1.0, threshold * 2.5);
        actionIntensity = std::max(0.38f, ClampUnit(static_cast<float>(speedAboveThreshold / speedWindow)));
    }
    if (outActionCode) {
        *outActionCode = actionCode;
    }
    if (outActionIntensity) {
        *outActionIntensity = actionIntensity;
    }
}

bool AppController::QueryCursorScreenPoint(ScreenPoint* outPt) const {
    if (!outPt || !cursorPositionService_) {
        return false;
    }
    return cursorPositionService_->TryGetCursorScreenPoint(outPt);
}

MouseCompanionPetRuntimeConfig AppController::BuildMouseCompanionPluginRuntimeConfig(
    const MouseCompanionConfig& cfg) const {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(cfg);
    MouseCompanionPetRuntimeConfig out{};
    out.enabled = cfg.enabled;
    out.useTestProfile = cfg.useTestProfile;
    out.sizePx = cfg.sizePx;
    out.offsetX = cfg.offsetX;
    out.offsetY = cfg.offsetY;
    out.absoluteX = cfg.absoluteX;
    out.absoluteY = cfg.absoluteY;
    out.pressLiftPx = activeConfig.pressLiftPx;
    out.smoothingPercent = activeConfig.smoothingPercent;
    out.followThresholdPx = activeConfig.followThresholdPx;
    out.releaseHoldMs = activeConfig.releaseHoldMs;
    out.clickStreakBreakMs = activeConfig.clickStreakBreakMs;
    out.headTintPerClick = static_cast<float>(activeConfig.headTintPerClick);
    out.headTintMax = static_cast<float>(activeConfig.headTintMax);
    out.headTintDecayPerSecond = static_cast<float>(activeConfig.headTintDecayPerSecond);
    out.positionMode = cfg.positionMode;
    out.targetMonitor = cfg.targetMonitor;
    out.edgeClampMode = cfg.edgeClampMode;
    return out;
}

void AppController::ResetMouseCompanionPluginHosts(
    const MouseCompanionConfig& cfg,
    uint64_t nowTickMs) {
    petPluginHostPhase0_.Reset(cfg.enabled, nowTickMs);
    petPluginHostV1_.Reset(BuildMouseCompanionPluginRuntimeConfig(cfg), nowTickMs);
    SyncMouseCompanionPluginPhase0Status();
}

void AppController::OnMouseCompanionPluginConfigChanged(
    const MouseCompanionConfig& cfg,
    uint64_t nowTickMs) {
    petPluginHostPhase0_.OnConfigChanged(cfg.enabled, nowTickMs);
    petPluginHostV1_.OnConfigChanged(BuildMouseCompanionPluginRuntimeConfig(cfg), nowTickMs);
    SyncMouseCompanionPluginPhase0Status();
}

void AppController::SyncMouseCompanionPluginPhase0Status() {
    const MouseCompanionPluginPhase0Snapshot snapshot = petPluginHostPhase0_.Snapshot();
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.pluginHostReady = snapshot.hostReady;
    mouseCompanionRuntimeStatus_.pluginHostPhase = snapshot.hostPhase;
    mouseCompanionRuntimeStatus_.activePluginId = snapshot.activePluginId;
    mouseCompanionRuntimeStatus_.activePluginVersion = snapshot.activePluginVersion;
    mouseCompanionRuntimeStatus_.engineApiVersion = snapshot.engineApiVersion;
    mouseCompanionRuntimeStatus_.compatibilityStatus = snapshot.compatibilityStatus;
    mouseCompanionRuntimeStatus_.fallbackReason = snapshot.fallbackReason;
    mouseCompanionRuntimeStatus_.lastPluginEvent = snapshot.lastEventName;
    mouseCompanionRuntimeStatus_.lastPluginEventTickMs = snapshot.lastEventTickMs;
    mouseCompanionRuntimeStatus_.pluginEventCount = snapshot.eventCount;
}

void AppController::RecordMouseCompanionPluginPhase0Input(const char* eventName) {
    petPluginHostPhase0_.OnInputEvent(eventName, CurrentTickMs());
    SyncMouseCompanionPluginPhase0Status();
}

void AppController::RecordMouseCompanionPluginInput(
    const char* eventName,
    const MouseCompanionPetInputEvent& event) {
    RecordMouseCompanionPluginPhase0Input(eventName);
    petPluginHostV1_.OnInput(event);
}

void AppController::CommitMouseCompanionPluginResolvedFrame(const MouseCompanionPetPoseFrame& frame) {
    petPluginHostV1_.CommitResolvedFrame(frame);
}

void AppController::DispatchPetMove(const ScreenPoint& pt) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    double dtSeconds = 1.0 / 60.0;
    if (petLastTickMs_ > 0 && nowTickMs > petLastTickMs_) {
        dtSeconds = static_cast<double>(nowTickMs - petLastTickMs_) / 1000.0;
        dtSeconds = std::clamp(dtSeconds, 1.0 / 240.0, 1.0 / 12.0);
    }
    petLastTickMs_ = nowTickMs;

    const ScreenPoint runtimePt = ResolvePetRuntimeCursorPoint(pt, dtSeconds, activeConfig.smoothingPercent);
    UpdatePetPointerMotion(runtimePt, nowTickMs);
    UpdatePetPrimaryPressTravel(pt);
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);

    int actionCode = kPetActionIdle;
    float intensity = 0.0f;
    if (ResolvePetVisualHoldState(activeConfig, nowTickMs, &intensity)) {
        actionCode = kPetActionHoldReact;
        if (holdButtonDown_ && petPrimaryPress_.active) {
            petPrimaryPress_.holdTriggered = true;
        }
    } else {
        ResolvePetContinuousAction(activeConfig, &actionCode, &intensity);
    }
    if (actionCode != kPetActionHoldReact &&
        holdButtonDown_ &&
        petPrimaryPress_.active &&
        petPrimaryPress_.maxTravelPx > kPetDragStartTravelPx) {
        actionCode = kPetActionDrag;
        intensity = ClampUnit(static_cast<float>(petPrimaryPress_.maxTravelPx / 32.0));
    }

    UpdatePetVisualState(runtimePt, actionCode, intensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::Move;
    event.actionHint = (actionCode == kPetActionFollow || actionCode == kPetActionDrag)
        ? MouseCompanionPetActionHint::Follow
        : MouseCompanionPetActionHint::Idle;
    event.pt = runtimePt;
    event.tickMs = nowTickMs;
    event.actionIntensity = intensity;
    RecordMouseCompanionPluginInput("move", event);
}

void AppController::DispatchPetScroll(const ScreenPoint& pt, int delta) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    petLastScrollTickMs_ = nowTickMs;
    petReleaseHoldUntilTickMs_ = 0;
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    const float amplitude = std::clamp(
        static_cast<float>(std::abs(delta)) / 120.0f,
        kPetScrollAmplitudeMin,
        kPetScrollAmplitudeMax);
    const float durationMaxSec = ResolvePetScrollMotionDurationMaxSec(activeConfig.useTestProfile);
    const float durationMinSec = ResolvePetScrollMotionDurationMinSec(activeConfig.useTestProfile);
    float durationSec = durationMaxSec;
    if (petVisualPoseRuntime_.lastScrollEventTickMs != 0 && nowTickMs > petVisualPoseRuntime_.lastScrollEventTickMs) {
        const uint64_t intervalMs = std::clamp<uint64_t>(
            nowTickMs - petVisualPoseRuntime_.lastScrollEventTickMs,
            kPetScrollFastIntervalMs,
            kPetScrollSlowIntervalMs);
        const float fastness =
            ClampUnit(static_cast<float>(kPetScrollSlowIntervalMs - intervalMs) /
                      static_cast<float>(kPetScrollSlowIntervalMs - kPetScrollFastIntervalMs));
        durationSec = durationMaxSec - (durationMaxSec - durationMinSec) * fastness;
    }
    petVisualPoseRuntime_.lastScrollEventTickMs = nowTickMs;
    if (petVisualPoseRuntime_.scrollFlapProgress >= 1.0f) {
        petVisualPoseRuntime_.scrollFlapProgress = 0.0f;
        petVisualPoseRuntime_.scrollFlapDurationSec = durationSec;
        petVisualPoseRuntime_.scrollFlapAmplitude = amplitude;
    } else {
        petVisualPoseRuntime_.pendingScrollFlapCount =
            std::min<uint32_t>(petVisualPoseRuntime_.pendingScrollFlapCount + 1, kPetScrollQueueMax);
        petVisualPoseRuntime_.queuedScrollFlapDurationSec = durationSec;
        petVisualPoseRuntime_.queuedScrollFlapAmplitude = std::max(
            petVisualPoseRuntime_.queuedScrollFlapAmplitude,
            amplitude);
        const float currentProgress = ClampUnit(
            petVisualPoseRuntime_.scrollFlapProgress);
        const float currentElapsedSec =
            currentProgress * std::max(0.001f, petVisualPoseRuntime_.scrollFlapDurationSec);
        petVisualPoseRuntime_.scrollFlapDurationSec = std::min(
            petVisualPoseRuntime_.scrollFlapDurationSec,
            durationSec);
        petVisualPoseRuntime_.scrollFlapAmplitude = std::max(
            petVisualPoseRuntime_.scrollFlapAmplitude,
            amplitude);
        petVisualPoseRuntime_.scrollFlapProgress = ClampUnit(
            currentElapsedSec / std::max(0.001f, petVisualPoseRuntime_.scrollFlapDurationSec));
    }
    const float intensity = ClampUnit(static_cast<float>(std::abs(delta)) / 120.0f);
    const float directionalIntensity = (delta < 0) ? -intensity : intensity;
    UpdatePetVisualState(pt, kPetActionScrollReact, directionalIntensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::Scroll;
    event.actionHint = MouseCompanionPetActionHint::Scroll;
    event.pt = pt;
    event.delta = delta;
    event.tickMs = nowTickMs;
    event.actionIntensity = intensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("scroll", event);
}

void AppController::DispatchPetClick(const ClickEvent& ev) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);

    const bool isPrimary = (ev.button == MouseButton::Left);
    const bool eligible = isPrimary && EvaluatePetPrimaryClickEligibility(nowTickMs);
    if (eligible) {
        RegisterPetClickStreakClick(nowTickMs, activeConfig);
        const PetVisualMotionProfile& visualProfile =
            ResolvePetVisualMotionProfile(activeConfig.useTestProfile);
        const float pulseBase = std::min(
            visualProfile.clickActionPulseMax,
            visualProfile.clickActionPulseBase +
                visualProfile.clickActionPulseStreakStep * static_cast<float>(std::max(0, petClickStreak_.streak - 1)));
        UpdatePetVisualState(
            ev.pt,
            kPetActionClickReact,
            ClampUnit(pulseBase),
            petClickStreak_.tintAmount);
    } else {
        int fallbackAction = kPetActionIdle;
        float fallbackIntensity = 0.0f;
        ResolvePetContinuousAction(activeConfig, &fallbackAction, &fallbackIntensity);
        UpdatePetVisualState(ev.pt, fallbackAction, fallbackIntensity, petClickStreak_.tintAmount);
    }
    petPrimaryPress_.releaseReady = false;
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::Click;
    event.actionHint = eligible ? MouseCompanionPetActionHint::Click : MouseCompanionPetActionHint::Idle;
    event.pt = ev.pt;
    event.button = static_cast<int>(ev.button);
    event.tickMs = nowTickMs;
    event.actionIntensity = eligible ? petClickStreak_.tintAmount : 0.0f;
    event.accepted = eligible;
    RecordMouseCompanionPluginInput(eligible ? "click_accept" : "click_reject", event);
}

void AppController::DispatchPetHoverStart(const ScreenPoint& pt) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    UpdatePetClickStreakDecay(CurrentTickMs(), activeConfig);
    int actionCode = kPetActionIdle;
    float actionIntensity = 0.0f;
    ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    UpdatePetVisualState(pt, actionCode, actionIntensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::HoverStart;
    event.actionHint = MouseCompanionPetActionHint::Idle;
    event.pt = pt;
    event.tickMs = CurrentTickMs();
    event.actionIntensity = actionIntensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("hover_start", event);
}

void AppController::DispatchPetHoverEnd(const ScreenPoint& pt) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    UpdatePetClickStreakDecay(CurrentTickMs(), activeConfig);
    int actionCode = kPetActionIdle;
    float actionIntensity = 0.0f;
    ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    UpdatePetVisualState(pt, actionCode, actionIntensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::HoverEnd;
    event.actionHint = MouseCompanionPetActionHint::Idle;
    event.pt = pt;
    event.tickMs = CurrentTickMs();
    event.actionIntensity = actionIntensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("hover_end", event);
}

void AppController::DispatchPetButtonDown(const ScreenPoint& pt, int button) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    petReleaseHoldUntilTickMs_ = 0;
    int actionCode = kPetActionIdle;
    float actionIntensity = 0.0f;
    ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    if (button == static_cast<int>(MouseButton::Left)) {
        petPrimaryPress_ = PetPrimaryPressState{};
        petPrimaryPress_.active = true;
        petPrimaryPress_.downPoint = pt;
        petPrimaryPress_.downTickMs = nowTickMs;
        // Keep primary down neutral; drag should only begin after pointer travel.
        ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    }
    UpdatePetVisualState(pt, actionCode, actionIntensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::ButtonDown;
    event.actionHint = MouseCompanionPetActionHint::Idle;
    event.pt = pt;
    event.button = button;
    event.tickMs = nowTickMs;
    event.actionIntensity = actionIntensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("button_down", event);
}

void AppController::DispatchPetHoldStart(const ScreenPoint& pt, int button, uint32_t holdMs) {
    if (button == static_cast<int>(MouseButton::Left) && petPrimaryPress_.active) {
        petPrimaryPress_.holdTriggered = true;
    }
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    const float intensity = ClampUnit(static_cast<float>(holdMs) / 300.0f);
    UpdatePetVisualState(pt, kPetActionHoldReact, intensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::HoldStart;
    event.actionHint = MouseCompanionPetActionHint::Hold;
    event.pt = pt;
    event.button = button;
    event.holdMs = holdMs;
    event.tickMs = nowTickMs;
    event.actionIntensity = intensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("hold_start", event);
}

void AppController::DispatchPetHoldUpdate(const ScreenPoint& pt, uint32_t holdMs) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    const float intensity = ClampUnit(static_cast<float>(holdMs) / 600.0f);
    UpdatePetVisualState(pt, kPetActionHoldReact, intensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::HoldUpdate;
    event.actionHint = MouseCompanionPetActionHint::Hold;
    event.pt = pt;
    event.holdMs = holdMs;
    event.tickMs = nowTickMs;
    event.actionIntensity = intensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("hold_update", event);
}

void AppController::DispatchPetButtonUp(const ScreenPoint& pt, int button) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    if (button == static_cast<int>(MouseButton::Left) && petPrimaryPress_.active) {
        petPrimaryPress_.releaseReady = true;
        petPrimaryPress_.releaseTickMs = nowTickMs;
        petPrimaryPress_.releasePressMs =
            static_cast<uint32_t>((nowTickMs >= petPrimaryPress_.downTickMs)
                                      ? (nowTickMs - petPrimaryPress_.downTickMs)
                                      : 0);
        petPrimaryPress_.releaseMaxTravelPx = petPrimaryPress_.maxTravelPx;
        if (petPrimaryPress_.holdTriggered) {
            const uint64_t releaseHoldMs = static_cast<uint64_t>(std::max(0, activeConfig.releaseHoldMs));
            petReleaseHoldUntilTickMs_ = releaseHoldMs > 0 ? (nowTickMs + releaseHoldMs) : 0;
        } else {
            petReleaseHoldUntilTickMs_ = 0;
        }
        petPrimaryPress_.active = false;
    }
    int actionCode = kPetActionIdle;
    float actionIntensity = 0.0f;
    if (ResolvePetVisualHoldState(activeConfig, nowTickMs, &actionIntensity)) {
        actionCode = kPetActionHoldReact;
    } else {
        ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    }
    UpdatePetVisualState(pt, actionCode, actionIntensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::ButtonUp;
    event.actionHint = MouseCompanionPetActionHint::Idle;
    event.pt = pt;
    event.button = button;
    event.tickMs = nowTickMs;
    event.actionIntensity = actionIntensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("button_up", event);
}

void AppController::DispatchPetHoldEnd(const ScreenPoint& pt) {
    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    int actionCode = kPetActionIdle;
    float actionIntensity = 0.0f;
    if (ResolvePetVisualHoldState(activeConfig, nowTickMs, &actionIntensity)) {
        actionCode = kPetActionHoldReact;
    } else {
        ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    }
    UpdatePetVisualState(pt, actionCode, actionIntensity, petClickStreak_.tintAmount);
    MouseCompanionPetInputEvent event{};
    event.kind = MouseCompanionPetInputKind::HoldEnd;
    event.actionHint = MouseCompanionPetActionHint::Idle;
    event.pt = pt;
    event.tickMs = nowTickMs;
    event.actionIntensity = actionIntensity;
    event.accepted = true;
    RecordMouseCompanionPluginInput("hold_end", event);
}

void AppController::TickPetVisualFrame() {
#if MFX_PLATFORM_MACOS
    if (!petVisualHostHandle_ || !config_.mouseCompanion.enabled) {
        return;
    }

    const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
    const uint64_t nowTickMs = CurrentTickMs();
    UpdatePetClickStreakDecay(nowTickMs, activeConfig);
    DecayPetPointerMotion(nowTickMs, activeConfig);

    ScreenPoint pt{};
    if (!QueryCursorScreenPoint(&pt) && !TryGetLastPointerPoint(&pt)) {
        pt.x = 0;
        pt.y = 0;
    } else {
        RememberLastPointerPoint(pt);
    }

    int actionCode = kPetActionIdle;
    float actionIntensity = 0.0f;
    if (ResolvePetVisualHoldState(activeConfig, nowTickMs, &actionIntensity)) {
        actionCode = kPetActionHoldReact;
        if (holdButtonDown_ && petPrimaryPress_.active) {
            petPrimaryPress_.holdTriggered = true;
        }
    } else {
        ResolvePetContinuousAction(activeConfig, &actionCode, &actionIntensity);
    }
    if (actionCode != kPetActionHoldReact &&
        holdButtonDown_ &&
        petPrimaryPress_.active &&
        petPrimaryPress_.maxTravelPx > kPetDragStartTravelPx) {
        actionCode = kPetActionDrag;
        actionIntensity = ClampUnit(static_cast<float>(petPrimaryPress_.maxTravelPx / 32.0));
    }

    UpdatePetVisualState(pt, actionCode, actionIntensity, petClickStreak_.tintAmount);
#else
    (void)this;
#endif
}

ScreenPoint AppController::ResolvePetRuntimeCursorPoint(
    const ScreenPoint& rawPt,
    double dtSeconds,
    int smoothingPercent) {
    const int clampedSmoothing = std::clamp(smoothingPercent, 0, 95);
    if (clampedSmoothing <= 0) {
        petHasSmoothedCursor_ = true;
        petSmoothedCursorX_ = static_cast<double>(rawPt.x);
        petSmoothedCursorY_ = static_cast<double>(rawPt.y);
        return rawPt;
    }

    if (!petHasSmoothedCursor_) {
        petHasSmoothedCursor_ = true;
        petSmoothedCursorX_ = static_cast<double>(rawPt.x);
        petSmoothedCursorY_ = static_cast<double>(rawPt.y);
        return rawPt;
    }

    const double frameScale = std::max(0.25, dtSeconds * 60.0);
    const double alpha = std::clamp((1.0 - static_cast<double>(clampedSmoothing) / 100.0) * frameScale, 0.03, 1.0);
    petSmoothedCursorX_ += (static_cast<double>(rawPt.x) - petSmoothedCursorX_) * alpha;
    petSmoothedCursorY_ += (static_cast<double>(rawPt.y) - petSmoothedCursorY_) * alpha;
    ScreenPoint out{};
    out.x = static_cast<int>(std::lround(petSmoothedCursorX_));
    out.y = static_cast<int>(std::lround(petSmoothedCursorY_));
    return out;
}

void AppController::ResetPetDispatchRuntimeState() {
    const uint64_t nowTickMs = CurrentTickMs();
    ResetMouseCompanionPluginHosts(config_.mouseCompanion, nowTickMs);
    petDragging_ = false;
    petLastTickMs_ = 0;
    petReleaseHoldUntilTickMs_ = 0;
    petHasSmoothedCursor_ = false;
    petSmoothedCursorX_ = 0.0;
    petSmoothedCursorY_ = 0.0;
    petHasLastDispatchPoint_ = false;
    petLastDispatchPoint_ = ScreenPoint{};
    petClickStreak_.streak = 0;
    petClickStreak_.lastClickTickMs = 0;
    petClickStreak_.lastUpdateTickMs = 0;
    petClickStreak_.tintAmount = 0.0f;
    petPointerMotion_ = PetPointerMotionState{};
    petVisualPoseRuntime_ = PetVisualPoseRuntimeState{};
    petPrimaryPress_ = PetPrimaryPressState{};
    petLastScrollTickMs_ = 0;
    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.lastActionCode = 0;
        mouseCompanionRuntimeStatus_.lastActionIntensity = 0.0f;
        mouseCompanionRuntimeStatus_.lastActionTickMs = nowTickMs;
        mouseCompanionRuntimeStatus_.lastActionName = "idle";
        mouseCompanionRuntimeStatus_.clickStreak = 0;
        mouseCompanionRuntimeStatus_.clickStreakTintAmount = 0.0f;
    }
}

void AppController::UpdatePetVisualState(const ScreenPoint& pt, int actionCode, float actionIntensity, float headTintAmount) {
    const float clampedIntensity = ClampUnit(actionIntensity);
    const float hostActionIntensity =
        (actionCode == kPetActionScrollReact) ? actionIntensity : clampedIntensity;
    const float clampedTint = ClampUnit(headTintAmount);
    const uint64_t nowTickMs = CurrentTickMs();
    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.lastActionCode = actionCode;
        mouseCompanionRuntimeStatus_.lastActionIntensity = clampedIntensity;
        mouseCompanionRuntimeStatus_.lastActionTickMs = nowTickMs;
        mouseCompanionRuntimeStatus_.lastActionName = ResolvePetActionName(actionCode);
        mouseCompanionRuntimeStatus_.clickStreakTintAmount = clampedTint;
    }

    petPluginHostV1_.Tick(nowTickMs, 0.0f);
    MouseCompanionPetPoseFrame pluginPoseFrame{};
    pluginPoseFrame.sampleTickMs = nowTickMs;
    pluginPoseFrame.actionName = ResolvePetActionName(actionCode);
    pluginPoseFrame.actionIntensity = clampedIntensity;
    pluginPoseFrame.headTintAmount = clampedTint;

#if MFX_PLATFORM_MACOS
    if (petVisualHostHandle_ && config_.mouseCompanion.enabled) {
        mfx_macos_mouse_companion_panel_update_v1(
            petVisualHostHandle_,
            actionCode,
            hostActionIntensity,
            clampedTint);
        const MouseCompanionConfig activeConfig = ResolveActiveMouseCompanionConfig(config_.mouseCompanion);
        const PetVisualMotionProfile& visualProfile =
            ResolvePetVisualMotionProfile(activeConfig.useTestProfile);
        mfx_macos_mouse_companion_panel_move_follow_v1(
            petVisualHostHandle_,
            pt.x,
            pt.y);

        if (EnsurePetVisualPoseBinding()) {
            if (petVisualPoseRuntime_.lastTickMs == 0 || nowTickMs <= petVisualPoseRuntime_.lastTickMs) {
                petVisualPoseRuntime_.lastTickMs = nowTickMs;
            } else {
                const uint64_t deltaMs = nowTickMs - petVisualPoseRuntime_.lastTickMs;
                petVisualPoseRuntime_.lastTickMs = nowTickMs;
                const float dt = static_cast<float>(std::clamp<uint64_t>(deltaMs, 0, 120)) / 1000.0f;
                petVisualPoseRuntime_.holdPulse =
                    std::max(0.0f, petVisualPoseRuntime_.holdPulse - dt * visualProfile.holdDecayPerSecond);
                if (actionCode == kPetActionFollow) {
                    const float walkRateHz =
                        visualProfile.followWalkRateMinHz +
                        (visualProfile.followWalkRateMaxHz - visualProfile.followWalkRateMinHz) * clampedIntensity;
                    petVisualPoseRuntime_.followWalkPhase += dt * std::max(0.0f, walkRateHz);
                    if (petVisualPoseRuntime_.followWalkPhase >= 1.0f) {
                        petVisualPoseRuntime_.followWalkPhase =
                            petVisualPoseRuntime_.followWalkPhase -
                            std::floor(petVisualPoseRuntime_.followWalkPhase);
                    }
                }
                if (petVisualPoseRuntime_.scrollFlapProgress < 1.0f) {
                    petVisualPoseRuntime_.scrollFlapProgress +=
                        dt / std::max(0.001f, petVisualPoseRuntime_.scrollFlapDurationSec);
                    while (petVisualPoseRuntime_.scrollFlapProgress >= 1.0f &&
                           petVisualPoseRuntime_.pendingScrollFlapCount > 0) {
                        petVisualPoseRuntime_.pendingScrollFlapCount -= 1;
                        petVisualPoseRuntime_.scrollFlapProgress = 0.0f;
                        petVisualPoseRuntime_.scrollFlapDurationSec =
                            petVisualPoseRuntime_.queuedScrollFlapDurationSec;
                        petVisualPoseRuntime_.scrollFlapAmplitude =
                            petVisualPoseRuntime_.queuedScrollFlapAmplitude;
                    }
                    if (petVisualPoseRuntime_.scrollFlapProgress >= 1.0f) {
                        petVisualPoseRuntime_.scrollFlapProgress = 1.0f;
                    }
                }
            }

            if (actionCode == kPetActionHoldReact) {
                petVisualPoseRuntime_.holdPulse =
                    std::max(petVisualPoseRuntime_.holdPulse, std::max(visualProfile.holdPulseFloor, clampedIntensity));
            }
            const float holdProfile = std::max(
                ClampUnit(petVisualPoseRuntime_.holdPulse),
                (actionCode == kPetActionHoldReact) ? clampedIntensity : 0.0f);
            float scrollProfile = 0.0f;
            float scrollFlap = 0.0f;
            float scrollAmpNorm = 0.0f;
            if (petVisualPoseRuntime_.scrollFlapProgress < 1.0f) {
                const float scrollT = ClampUnit(petVisualPoseRuntime_.scrollFlapProgress);
                scrollAmpNorm = ClampUnit(
                    (petVisualPoseRuntime_.scrollFlapAmplitude - kPetScrollAmplitudeMin) /
                    (kPetScrollAmplitudeMax - kPetScrollAmplitudeMin));
                scrollProfile = ClampUnit(
                    ImpulseProfile(scrollT, 0.42f, 0.16f) * (0.82f + scrollAmpNorm * 0.18f));
                scrollFlap = std::sin(scrollT * 3.1415926f) *
                             scrollProfile *
                             (0.28f + scrollAmpNorm * 0.16f);
            }
            const float holdTerm = ClampUnit(holdProfile * visualProfile.holdPoseGain);
            const float scrollTerm = ClampUnit(scrollProfile * visualProfile.scrollPoseGain);
            const float followProfile = (actionCode == kPetActionFollow) ? clampedIntensity : 0.0f;
            const float followTerm = ClampUnit(followProfile * visualProfile.followPoseGain);
            const float walkPhase = ClampUnit(petVisualPoseRuntime_.followWalkPhase) * 6.2831853f;
            const float walkSwing = std::sin(walkPhase);
            const float earWingOpen = ClampUnit(
                scrollTerm * (0.72f + scrollAmpNorm * 0.12f) + std::abs(scrollFlap) * 0.10f);
            const float earLift = ClampUnit(
                scrollTerm * 0.36f + std::abs(scrollFlap) * 0.05f);
            const float earWingFlap = scrollFlap * (0.78f + scrollAmpNorm * 0.12f);
            const float handLift = ClampUnit(
                holdTerm * 0.08f +
                scrollTerm * 0.68f +
                std::abs(scrollFlap) * 0.10f);
            const float handSpread = ClampUnit(
                holdTerm * 0.16f +
                scrollTerm * 0.24f);
            const float handInward = ClampUnit(holdTerm * 0.72f);
            const float legSpread = ClampUnit(
                scrollTerm * 0.70f);
            const float legInward = ClampUnit(holdTerm * 0.58f);
            const float legCurl = ClampUnit(
                holdTerm * 0.96f +
                scrollTerm * 0.40f);

            std::array<int32_t, kPetPoseBoneCount> boneIndices = {
                kPetBoneLeftEar,
                kPetBoneRightEar,
                kPetBoneLeftHand,
                kPetBoneRightHand,
                kPetBoneLeftLeg,
                kPetBoneRightLeg,
            };
            std::array<float, kPetPoseBoneCount * 3> positions{};
            std::array<float, kPetPoseBoneCount * 4> rotations{};
            std::array<float, kPetPoseBoneCount * 3> scales{};
            for (int i = 0; i < kPetPoseBoneCount; ++i) {
                scales[static_cast<size_t>(i) * 3 + 0] = 1.0f;
                scales[static_cast<size_t>(i) * 3 + 1] = 1.0f;
                scales[static_cast<size_t>(i) * 3 + 2] = 1.0f;
            }

            positions[0] = -earWingOpen * 0.19f;
            positions[1] = earLift * 0.14f;
            positions[3] = earWingOpen * 0.19f;
            positions[4] = earLift * 0.14f;
            positions[6] = handInward * 0.04f - handSpread * 0.10f;
            positions[7] = handLift * 0.04f - holdTerm * 0.05f + followTerm * std::max(0.0f, -walkSwing) * 0.020f;
            positions[8] = -holdTerm * 0.012f;
            positions[9] = -handInward * 0.04f + handSpread * 0.10f;
            positions[10] = handLift * 0.04f - holdTerm * 0.05f + followTerm * std::max(0.0f, walkSwing) * 0.020f;
            positions[11] = -holdTerm * 0.012f;
            positions[12] = legInward * 0.08f - legSpread * 0.17f;
            positions[13] = holdTerm * 0.05f + followTerm * std::max(0.0f, walkSwing) * 0.010f;
            positions[15] = -legInward * 0.08f + legSpread * 0.17f;
            positions[16] = holdTerm * 0.05f + followTerm * std::max(0.0f, -walkSwing) * 0.010f;

            float q[4]{};
            WriteQuaternionFromEuler(
                -1.02f * holdTerm - 0.04f * scrollTerm + 0.03f * std::abs(earWingFlap),
                0.04f * earWingOpen,
                (0.09f * holdTerm + 0.14f * scrollProfile + 0.20f * earWingFlap + 0.44f * earWingOpen),
                q);
            rotations[0] = q[0];
            rotations[1] = q[1];
            rotations[2] = q[2];
            rotations[3] = q[3];
            WriteQuaternionFromEuler(
                -1.02f * holdTerm - 0.04f * scrollTerm + 0.03f * std::abs(earWingFlap),
                -0.04f * earWingOpen,
                -(0.09f * holdTerm + 0.14f * scrollProfile + 0.20f * earWingFlap + 0.44f * earWingOpen),
                q);
            rotations[4] = q[0];
            rotations[5] = q[1];
            rotations[6] = q[2];
            rotations[7] = q[3];
            WriteQuaternionFromEuler(
                -0.42f * holdTerm,
                0.44f * holdTerm,
                0.30f * holdTerm + 0.20f * scrollProfile + 0.26f * scrollFlap + 0.08f * handSpread +
                    followTerm * (0.06f - 0.18f * walkSwing),
                q);
            rotations[8] = q[0];
            rotations[9] = q[1];
            rotations[10] = q[2];
            rotations[11] = q[3];
            WriteQuaternionFromEuler(
                -0.42f * holdTerm,
                -0.44f * holdTerm,
                -(0.30f * holdTerm + 0.20f * scrollProfile - 0.26f * scrollFlap + 0.08f * handSpread +
                    followTerm * (0.06f + 0.18f * walkSwing)),
                q);
            rotations[12] = q[0];
            rotations[13] = q[1];
            rotations[14] = q[2];
            rotations[15] = q[3];
            WriteQuaternionFromEuler(0.0f, 0.0f, -0.42f * legCurl + followTerm * (0.04f + 0.16f * walkSwing), q);
            rotations[16] = q[0];
            rotations[17] = q[1];
            rotations[18] = q[2];
            rotations[19] = q[3];
            WriteQuaternionFromEuler(0.0f, 0.0f, 0.42f * legCurl + followTerm * (0.04f - 0.16f * walkSwing), q);
            rotations[20] = q[0];
            rotations[21] = q[1];
            rotations[22] = q[2];
            rotations[23] = q[3];

            pluginPoseFrame.samples.reserve(kPetPoseBoneCount);
            for (int i = 0; i < kPetPoseBoneCount; ++i) {
                MouseCompanionPetPoseSample sample{};
                sample.boneIndex = boneIndices[static_cast<size_t>(i)];
                const size_t pBase = static_cast<size_t>(i) * 3;
                const size_t rBase = static_cast<size_t>(i) * 4;
                sample.position[0] = positions[pBase + 0];
                sample.position[1] = positions[pBase + 1];
                sample.position[2] = positions[pBase + 2];
                sample.rotation[0] = rotations[rBase + 0];
                sample.rotation[1] = rotations[rBase + 1];
                sample.rotation[2] = rotations[rBase + 2];
                sample.rotation[3] = rotations[rBase + 3];
                sample.scale[0] = scales[pBase + 0];
                sample.scale[1] = scales[pBase + 1];
                sample.scale[2] = scales[pBase + 2];
                pluginPoseFrame.samples.push_back(sample);
            }

            mfx_macos_mouse_companion_panel_apply_pose_v1(
                petVisualHostHandle_,
                boneIndices.data(),
                positions.data(),
                rotations.data(),
                scales.data(),
                kPetPoseBoneCount);
        }
    }
#else
    (void)pt;
#endif

    CommitMouseCompanionPluginResolvedFrame(pluginPoseFrame);
    petPluginHostV1_.SamplePose(&pluginPoseFrame);
}

bool AppController::EnsurePetVisualPoseBinding() {
#if MFX_PLATFORM_MACOS
    if (!petVisualHostHandle_) {
        petVisualPoseBindingAttempted_ = false;
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
        mouseCompanionRuntimeStatus_.skeletonBoneCount = 0;
        return false;
    }
    if (!petVisualPoseBindingAttempted_) {
        petVisualPoseBindingAttempted_ = true;
        petVisualPoseBindingConfigured_ = mfx_macos_mouse_companion_panel_configure_pose_binding_v1(
            petVisualHostHandle_,
            kPetPoseBoneNames.data(),
            kPetPoseBoneCount);
    }
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.poseBindingConfigured = petVisualPoseBindingConfigured_;
    mouseCompanionRuntimeStatus_.skeletonBoneCount = petVisualPoseBindingConfigured_ ? kPetPoseBoneCount : 0;
    return petVisualPoseBindingConfigured_;
#else
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
    mouseCompanionRuntimeStatus_.skeletonBoneCount = 0;
    return false;
#endif
}

void AppController::TryApplyPetAppearanceToVisualHost() {
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

std::string AppController::CurrentForegroundProcessBaseName() const {
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
