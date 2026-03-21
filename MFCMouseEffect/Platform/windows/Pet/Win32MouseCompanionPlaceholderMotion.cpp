#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"

#include "MouseFx/Utils/TimeUtils.h"

#include <cmath>

namespace mousefx::windows {
namespace {

constexpr double kTau = 6.28318530717958647692;

float ClampUnit(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

double ResolveSecondsPhase(uint64_t nowMs, double frequencyHz, double phaseOffset = 0.0) {
    const double t = static_cast<double>(nowMs) / 1000.0;
    return t * frequencyHz * kTau + phaseOffset;
}

float ResolveActionWeight(const Win32MouseCompanionVisualState& state, const char* actionName) {
    if (!actionName) {
        return 0.0f;
    }
    return state.lastActionName == actionName ? ClampUnit(state.lastActionIntensity) : 0.0f;
}

} // namespace

Win32MouseCompanionPlaceholderMotion BuildWin32MouseCompanionPlaceholderMotion(
    const Win32MouseCompanionVisualState& state) {
    Win32MouseCompanionPlaceholderMotion motion{};

    const uint64_t nowMs = NowMs();
    motion.reactive = BuildWin32MouseCompanionReactiveMotion(state, nowMs);
    const float followWeight = ResolveActionWeight(state, "follow");
    const float dragWeight = ResolveActionWeight(state, "drag");
    const float holdWeight = ResolveActionWeight(state, "hold_react");
    const float scrollWeight = ResolveActionWeight(state, "scroll_react");
    const float clickWeight = ResolveActionWeight(state, "click_react");
    const float moveWeight = followWeight + dragWeight * 0.85f;
    const float calmWeight = 1.0f - ClampUnit(moveWeight + holdWeight * 0.55f + scrollWeight * 0.35f);

    const double idlePhase = ResolveSecondsPhase(nowMs, 0.82);
    const double followPhase = ResolveSecondsPhase(nowMs, 1.65);
    const double blinkPhase = ResolveSecondsPhase(nowMs, 0.36, 0.8);
    const double tailPhase = ResolveSecondsPhase(nowMs, 1.05, 1.6);

    motion.bodyBobPx =
        static_cast<float>(std::sin(idlePhase) * (1.2 + calmWeight * 1.4) +
                           std::sin(followPhase) * moveWeight * 1.4) +
        motion.reactive.clickRebound * 2.2f -
        motion.reactive.holdCompression * 1.8f +
        motion.reactive.scrollKick * motion.reactive.scrollDirection * 0.4f;
    motion.headBobPx =
        static_cast<float>(std::sin(idlePhase + 0.8) * (0.8 + calmWeight * 1.1) +
                           std::sin(followPhase + 0.4) * moveWeight * 1.0) -
        motion.reactive.clickImpact * 2.0f +
        motion.reactive.clickRebound * 1.1f -
        motion.reactive.scrollKick * 0.6f;
    motion.headSwayPx =
        static_cast<float>(std::sin(followPhase * 0.5) * moveWeight * 2.4) +
        static_cast<float>(std::sin(idlePhase * 0.6) * calmWeight * 0.6) +
        motion.reactive.scrollDirection * motion.reactive.scrollKick * 4.0f;

    motion.shadowScaleX =
        1.0f + moveWeight * 0.08f - holdWeight * 0.04f +
        motion.reactive.clickImpact * 0.10f + motion.reactive.scrollKick * 0.05f;
    motion.shadowScaleY =
        1.0f - moveWeight * 0.05f + holdWeight * 0.05f -
        motion.reactive.clickImpact * 0.08f + motion.reactive.clickRebound * 0.05f;
    motion.tailLiftPx =
        static_cast<float>(std::sin(idlePhase + 1.4) * (1.8 + calmWeight * 1.8) +
                           std::sin(followPhase + 1.1) * moveWeight * 1.7) -
        holdWeight * 1.6f +
        motion.reactive.clickRebound * 2.6f -
        motion.reactive.holdCompression * 1.0f;
    motion.tailSwingPx =
        static_cast<float>(std::sin(tailPhase) * (2.4 + calmWeight * 1.0 + moveWeight * 2.2)) +
        motion.reactive.scrollDirection * motion.reactive.scrollKick * 3.4f;

    const float frontScale = (state.facingDirection < 0) ? 1.0f : 0.82f;
    const float rearScale = (state.facingDirection < 0) ? 0.82f : 1.0f;
    motion.frontEarLiftPx =
        static_cast<float>(std::sin(followPhase + 0.2) * (2.2 + moveWeight * 3.5) +
                           std::sin(idlePhase + 0.3) * calmWeight * 1.5) * frontScale +
        motion.reactive.attentionFocus * 2.8f;
    motion.rearEarLiftPx =
        static_cast<float>(std::sin(followPhase - 0.35) * (1.4 + moveWeight * 2.4) +
                           std::sin(idlePhase - 0.1) * calmWeight * 0.9) * rearScale +
        motion.reactive.attentionFocus * 1.4f;
    motion.frontEarSwingPx =
        static_cast<float>(std::sin(followPhase + 1.0) * (1.5 + moveWeight * 2.6)) * frontScale +
        motion.reactive.scrollDirection * motion.reactive.scrollKick * 2.2f;
    motion.rearEarSwingPx =
        static_cast<float>(std::sin(followPhase + 0.2) * (1.0 + moveWeight * 1.8)) * rearScale +
        motion.reactive.scrollDirection * motion.reactive.scrollKick * 1.4f;

    motion.frontPawLiftPx =
        static_cast<float>(std::sin(followPhase + 0.1) * (2.0 + moveWeight * 3.8)) +
        dragWeight * 1.8f - holdWeight * 1.2f +
        motion.reactive.clickImpact * 1.8f - motion.reactive.holdCompression * 1.0f;
    motion.rearPawLiftPx =
        static_cast<float>(std::sin(followPhase + 3.2) * (1.5 + moveWeight * 2.5)) +
        dragWeight * 0.8f + motion.reactive.clickRebound * 1.4f;
    motion.frontLegLiftPx =
        static_cast<float>(std::sin(followPhase + 0.6) * (1.6 + moveWeight * 3.2)) +
        motion.reactive.dragTension * 1.4f;
    motion.rearLegLiftPx =
        static_cast<float>(std::sin(followPhase + 3.6) * (1.2 + moveWeight * 2.1)) +
        motion.reactive.scrollKick * 0.8f;

    const double blinkWave = 0.5 + 0.5 * std::sin(blinkPhase);
    motion.blinkAmount =
        static_cast<float>(std::pow(blinkWave, 18.0)) * (0.45f + calmWeight * 0.40f) +
        clickWeight * 0.10f + motion.reactive.attentionFocus * 0.08f;
    motion.blinkAmount = ClampUnit(motion.blinkAmount);

    motion.breathScaleY =
        1.0f + static_cast<float>(std::sin(idlePhase + 0.4) * 0.025 * (0.4 + calmWeight)) -
        holdWeight * 0.03f + scrollWeight * 0.015f -
        motion.reactive.holdCompression * 0.035f +
        motion.reactive.clickRebound * 0.020f;
    motion.bodyScaleX =
        1.0f + motion.reactive.clickImpact * 0.11f +
        motion.reactive.dragTension * 0.06f +
        motion.reactive.scrollKick * 0.03f;
    motion.bodyScaleY =
        1.0f - motion.reactive.clickImpact * 0.09f -
        motion.reactive.holdCompression * 0.08f +
        motion.reactive.clickRebound * 0.04f;
    motion.headScaleX =
        1.0f + motion.reactive.attentionFocus * 0.04f +
        motion.reactive.scrollKick * 0.03f;
    motion.headScaleY =
        1.0f - motion.reactive.clickImpact * 0.07f -
        motion.reactive.holdCompression * 0.05f;
    motion.bodyTiltDeg =
        motion.reactive.scrollDirection * motion.reactive.scrollKick * 7.5f +
        motion.reactive.dragTension * static_cast<float>(state.facingDirection) * 2.0f;
    motion.eyeOpenScale =
        1.0f - motion.reactive.attentionFocus * 0.20f - motion.blinkAmount * 0.65f;
    motion.mouthOpenPx = motion.reactive.mouthOpen * 5.5f;
    motion.cheekLiftPx = motion.reactive.cheekLift * 2.6f;
    motion.chestBobPx =
        motion.reactive.holdPulse * 1.8f +
        motion.reactive.clickRebound * 1.2f -
        motion.reactive.clickImpact * 0.8f;
    motion.glowBoostAlpha = motion.reactive.glowBoost * 36.0f;
    motion.tailWidthScale =
        1.0f + motion.reactive.dragTension * 0.10f + motion.reactive.clickImpact * 0.08f;
    motion.whiskerSpreadPx =
        motion.reactive.attentionFocus * 2.2f +
        motion.reactive.scrollKick * 1.6f;
    return motion;
}

} // namespace mousefx::windows
