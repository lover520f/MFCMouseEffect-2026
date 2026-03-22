#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererMotionProfile.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

float Clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float ClampSigned(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

Gdiplus::Color MakeColor(BYTE a, BYTE r, BYTE g, BYTE b) {
    return Gdiplus::Color(a, r, g, b);
}

float TimePhaseRadians(uint64_t tickMs, float periodMs) {
    if (periodMs <= 1.0f) {
        return 0.0f;
    }
    const float wrapped = std::fmod(static_cast<float>(tickMs), periodMs);
    return wrapped / periodMs * 6.2831853f;
}

float PickBlushAlpha(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    float reactiveIntensity) {
    if (runtime.click) {
        return 190.0f;
    }
    if (runtime.hold) {
        return 120.0f;
    }
    return 130.0f + reactiveIntensity * 42.0f;
}

} // namespace

Win32MouseCompanionRealRendererMotionProfile BuildWin32MouseCompanionRealRendererMotionProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererMotionProfile profile{};
    profile.actionIntensity = Clamp01(runtime.actionIntensity);
    profile.reactiveIntensity = Clamp01(runtime.reactiveActionIntensity);
    profile.scrollIntensity = std::abs(ClampSigned(runtime.scrollSignedIntensity));

    const float breathePhase = TimePhaseRadians(runtime.poseSampleTickMs, 2100.0f);
    const float gesturePhaseFast = TimePhaseRadians(runtime.poseSampleTickMs, 720.0f);
    const float gesturePhaseMedium = TimePhaseRadians(runtime.poseSampleTickMs, 960.0f);
    const float gaitPhase = TimePhaseRadians(runtime.poseSampleTickMs, 820.0f);
    const float idleBreath = std::sin(breathePhase);
    const float idleSway = std::sin(breathePhase * 0.7f + 0.9f);
    const float idlePulse = runtime.click || runtime.hold || runtime.scroll || runtime.drag || runtime.follow
        ? 0.0f
        : 1.0f;

    profile.clickSquash = runtime.click ? (0.08f + profile.actionIntensity * 0.12f) : 0.0f;
    profile.dragLean = runtime.drag ? runtime.facingSign * (5.0f + profile.actionIntensity * 6.5f) : 0.0f;
    profile.scrollLean = runtime.scrollSignedIntensity * 8.5f;
    profile.earLift = (runtime.follow ? 10.0f + profile.actionIntensity * 5.0f : 2.5f) +
        (runtime.scroll ? profile.scrollIntensity * 12.0f : 0.0f);
    profile.earSwing =
        runtime.facingSign * (runtime.follow ? 6.0f + profile.actionIntensity * 4.5f : 2.0f);
    profile.earSpreadPulse = 0.0f;
    profile.handLift =
        runtime.hold ? (13.0f + profile.actionIntensity * 11.0f) : (runtime.follow ? 3.5f : 0.0f);
    profile.handSwing = 0.0f;
    profile.legStride = runtime.follow ? (7.0f + profile.actionIntensity * 7.0f) : 0.0f;
    profile.legLift = 0.0f;
    profile.stateLift = runtime.click ? (8.0f + profile.actionIntensity * 6.0f)
        : runtime.follow             ? (4.5f + profile.actionIntensity * 3.5f)
        : runtime.scroll             ? (2.0f + profile.scrollIntensity * 2.5f)
                                     : 0.0f;
    profile.headNod = runtime.follow ? (-4.5f - profile.actionIntensity * 3.5f)
        : runtime.hold              ? (2.5f + profile.actionIntensity * 2.2f)
        : runtime.scroll            ? (-runtime.scrollSignedIntensity * 4.0f)
        : runtime.drag              ? (-runtime.facingSign * 2.1f)
                                    : 0.0f;
    profile.bodyForward = runtime.drag ? runtime.facingSign * (7.0f + profile.actionIntensity * 5.0f)
        : runtime.follow               ? runtime.facingSign * (3.5f + profile.actionIntensity * 3.0f)
        : runtime.scroll               ? runtime.scrollSignedIntensity * 2.3f
                                       : 0.0f;
    profile.tailLift = runtime.follow ? (-7.0f - profile.actionIntensity * 4.5f)
        : runtime.click               ? (-4.0f - profile.actionIntensity * 2.5f)
        : runtime.hold                ? 4.5f
        : runtime.scroll              ? runtime.scrollSignedIntensity * 4.5f
                                      : 0.0f;
    profile.tailSwing = 0.0f;
    profile.shadowScale = runtime.click ? 0.86f : runtime.follow ? 0.92f : runtime.hold ? 1.08f : 1.0f;

    profile.breathLift = idlePulse * (idleBreath * 2.2f);
    profile.breathScale = 1.0f + idlePulse * (idleBreath * 0.014f);
    profile.idleTailSway = idlePulse * idleSway * 5.0f;
    profile.idleEarCadence = idlePulse * idleBreath * 3.0f;
    profile.idleHeadSway = idlePulse * idleSway * 1.6f;
    profile.idleHandFloat = idlePulse * std::sin(breathePhase * 1.15f + 0.6f) * 1.8f;

    if (runtime.click) {
        const float rebound = std::max(0.0f, std::sin(gesturePhaseFast));
        profile.clickSquash += rebound * 0.04f;
        profile.stateLift += rebound * 1.8f;
        profile.headNod -= rebound * 1.3f;
        profile.tailLift -= rebound * 0.9f;
        profile.tailSwing += runtime.facingSign * rebound * 1.2f;
    }
    if (runtime.hold) {
        const float squeeze = std::sin(gesturePhaseMedium + 0.4f);
        profile.handLift += squeeze * 2.2f;
        profile.handSwing += runtime.facingSign * squeeze * 0.8f;
        profile.headNod += squeeze * 1.1f;
        profile.shadowScale += std::max(0.0f, squeeze) * 0.04f;
        profile.earSpreadPulse += squeeze * 0.9f;
    }
    if (runtime.scroll) {
        const float swirl = std::sin(gesturePhaseMedium * 1.15f + runtime.scrollSignedIntensity * 0.8f);
        profile.stateLift += swirl * profile.scrollIntensity * 1.4f;
        profile.headNod -= swirl * profile.scrollIntensity * 1.1f;
        profile.tailLift += swirl * profile.scrollIntensity * 1.6f;
        profile.tailSwing += runtime.scrollSignedIntensity * swirl * 1.5f;
        profile.earSpreadPulse += swirl * profile.scrollIntensity * 0.8f;
    }
    if (runtime.drag) {
        const float dragPulse = std::sin(gesturePhaseFast * 0.8f + 0.5f);
        profile.dragLean += runtime.facingSign * dragPulse * 1.3f;
        profile.bodyForward += runtime.facingSign * std::max(0.0f, dragPulse) * 1.2f;
        profile.handSwing += runtime.facingSign * dragPulse * 1.4f;
    }
    if (runtime.follow) {
        const float gaitWave = std::sin(gaitPhase);
        const float gaitCounter = std::sin(gaitPhase + 1.5707963f);
        profile.legStride += gaitWave * 2.4f;
        profile.legLift += std::abs(gaitCounter) * (1.6f + profile.actionIntensity * 0.8f);
        profile.bodyForward += runtime.facingSign * gaitWave * 1.4f;
        profile.headNod += gaitCounter * 1.3f;
        profile.tailLift += gaitWave * 1.1f;
        profile.tailSwing += runtime.facingSign * gaitCounter * 1.8f;
        profile.earSwing += runtime.facingSign * gaitCounter * 1.2f;
        profile.handSwing += runtime.facingSign * gaitCounter * 1.1f;
        profile.earSpreadPulse += gaitWave * 0.7f;
    }

    profile.overlayAccentColor = runtime.click ? MakeColor(230, 255, 118, 186)
        : runtime.hold                         ? MakeColor(220, 255, 180, 104)
        : runtime.scroll                       ? MakeColor(220, 99, 213, 255)
        : runtime.drag                         ? MakeColor(210, 167, 134, 255)
        : runtime.follow                       ? MakeColor(210, 126, 228, 185)
                                               : MakeColor(180, 111, 219, 255);
    profile.blushAlpha = PickBlushAlpha(runtime, profile.reactiveIntensity);
    profile.eyeOpen = runtime.hold ? 0.28f : (runtime.click ? 0.42f : 1.0f);
    profile.pupilFocusX = runtime.drag ? runtime.facingSign * 0.95f
        : runtime.follow               ? runtime.facingSign * 0.72f
        : runtime.scroll               ? runtime.scrollSignedIntensity * 0.60f
        : runtime.click                ? runtime.facingSign * 0.18f
                                       : runtime.facingSign * 0.08f;
    profile.pupilFocusY = runtime.click ? -0.30f
        : runtime.hold                 ? 0.32f
        : runtime.follow               ? -0.18f
        : runtime.scroll               ? -runtime.scrollSignedIntensity * 0.14f
                                       : -0.06f;
    profile.eyeHighlightAlpha = runtime.hold ? 110.0f
        : runtime.drag                      ? 150.0f
        : runtime.click                     ? 235.0f
        : runtime.follow                    ? 205.0f
        : runtime.scroll                    ? 190.0f
                                            : 182.0f;
    profile.browTilt = runtime.drag ? runtime.facingSign * 4.5f
        : runtime.scroll            ? runtime.scrollSignedIntensity * 5.5f
        : runtime.click             ? runtime.facingSign * 1.6f
                                    : 0.0f;
    profile.browLift = runtime.follow ? -3.2f : (runtime.hold ? 2.1f : 0.0f);

    if (runtime.hold) {
        profile.mouthStartDeg = 24.0f;
        profile.mouthSweepDeg = 132.0f;
        profile.mouthStrokeWidth = 1.7f;
    } else if (runtime.click) {
        profile.mouthStartDeg = 0.0f;
        profile.mouthSweepDeg = 180.0f;
        profile.mouthStrokeWidth = 1.8f;
    } else if (runtime.scroll) {
        profile.mouthStartDeg = runtime.scrollSignedIntensity >= 0.0f ? 210.0f : 330.0f;
        profile.mouthSweepDeg = 120.0f;
        profile.mouthStrokeWidth = 1.5f;
    } else if (runtime.drag) {
        profile.mouthStartDeg = 26.0f;
        profile.mouthSweepDeg = 118.0f;
        profile.mouthStrokeWidth = 1.6f;
    } else if (runtime.follow) {
        profile.mouthStartDeg = 6.0f;
        profile.mouthSweepDeg = 170.0f;
        profile.mouthStrokeWidth = 1.5f;
    }

    return profile;
}

} // namespace mousefx::windows
