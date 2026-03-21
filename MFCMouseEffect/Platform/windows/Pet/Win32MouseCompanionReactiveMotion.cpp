#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionReactiveMotion.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

constexpr double kTau = 6.28318530717958647692;
constexpr float kClickWindowMs = 260.0f;
constexpr float kScrollWindowMs = 240.0f;

float ClampUnit(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float ClampSignedUnit(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

float SafeElapsedMs(uint64_t nowMs, uint64_t tickMs) {
    if (tickMs == 0 || nowMs <= tickMs) {
        return 0.0f;
    }
    return static_cast<float>(nowMs - tickMs);
}

float EaseOutCubic(float t) {
    const float clamped = ClampUnit(t);
    const float inv = 1.0f - clamped;
    return 1.0f - inv * inv * inv;
}

float DecayWindow(float elapsedMs, float durationMs) {
    if (durationMs <= 0.0f) {
        return 0.0f;
    }
    const float t = ClampUnit(elapsedMs / durationMs);
    return 1.0f - EaseOutCubic(t);
}

float SinePulse(float elapsedMs, float durationMs) {
    if (durationMs <= 0.0f) {
        return 0.0f;
    }
    const float t = ClampUnit(elapsedMs / durationMs);
    return static_cast<float>(std::sin(static_cast<double>(t) * kTau * 0.5));
}

float ResolveActionWeight(const Win32MouseCompanionVisualState& state, const char* actionName) {
    if (!actionName) {
        return 0.0f;
    }
    return state.lastActionName == actionName ? ClampUnit(state.lastActionIntensity) : 0.0f;
}

} // namespace

Win32MouseCompanionReactiveMotion BuildWin32MouseCompanionReactiveMotion(
    const Win32MouseCompanionVisualState& state,
    uint64_t nowMs) {
    Win32MouseCompanionReactiveMotion motion{};

    const float clickWeight = ResolveActionWeight(state, "click_react");
    const float dragWeight = ResolveActionWeight(state, "drag");
    const float holdWeight = ResolveActionWeight(state, "hold_react");
    const float scrollWeight = ResolveActionWeight(state, "scroll_react");

    const float clickElapsedMs = SafeElapsedMs(nowMs, state.lastClickTriggerTickMs);
    const float scrollElapsedMs = SafeElapsedMs(nowMs, state.lastScrollTriggerTickMs);
    const float holdElapsedMs = SafeElapsedMs(nowMs, state.lastHoldTriggerTickMs);

    const float clickPulse = DecayWindow(clickElapsedMs, kClickWindowMs);
    const float clickShape = SinePulse(clickElapsedMs, kClickWindowMs);
    motion.clickImpact = std::max(clickWeight, clickPulse) * (0.70f + clickShape * 0.30f);
    motion.clickRebound = clickPulse * (1.0f - clickShape) * 0.75f;

    const float holdRamp = ClampUnit(holdElapsedMs / 180.0f);
    motion.holdCompression = holdWeight * (0.55f + EaseOutCubic(holdRamp) * 0.45f);
    if (holdWeight > 0.0f) {
        const double holdPhase = (static_cast<double>(nowMs) / 1000.0) * 5.4;
        motion.holdPulse =
            static_cast<float>(std::sin(holdPhase) * 0.5 + 0.5) * holdWeight;
    }

    const float scrollPulse = DecayWindow(scrollElapsedMs, kScrollWindowMs);
    const float scrollDirection = ClampSignedUnit(state.lastScrollSignedIntensity);
    motion.scrollDirection = scrollDirection;
    motion.scrollKick = std::max(scrollWeight, scrollPulse) * (0.65f + std::abs(scrollDirection) * 0.35f);

    motion.dragTension = dragWeight * 0.85f + clickWeight * 0.20f;
    motion.attentionFocus = std::max(
        std::max(clickWeight, holdWeight * 0.92f),
        std::max(scrollWeight * 0.70f, dragWeight * 0.55f));
    motion.cheekLift =
        motion.clickImpact * 0.80f +
        motion.scrollKick * 0.35f +
        motion.holdCompression * 0.45f;
    motion.mouthOpen =
        motion.clickImpact * 0.75f +
        motion.scrollKick * 0.25f +
        motion.dragTension * 0.20f;
    motion.glowBoost =
        motion.clickImpact * 0.90f +
        motion.scrollKick * 0.80f +
        motion.holdCompression * 0.35f;
    return motion;
}

} // namespace mousefx::windows
