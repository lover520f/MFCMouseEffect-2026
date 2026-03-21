#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderActionProfile.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

float ClampUnit(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float ClampSigned(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

} // namespace

Win32MouseCompanionPlaceholderActionProfile BuildWin32MouseCompanionPlaceholderActionProfile(
    const Win32MouseCompanionVisualState& state,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Win32MouseCompanionPlaceholderPosture& posture,
    float facingSign) {
    Win32MouseCompanionPlaceholderActionProfile profile{};

    const float intensity = ClampUnit(state.lastActionIntensity);
    const float signedIntensity = ClampSigned(state.lastActionIntensity);
    const bool follow = state.lastActionName == "follow";
    const bool drag = state.lastActionName == "drag";
    const bool hold = state.lastActionName == "hold_react";
    const bool scroll = state.lastActionName == "scroll_react";
    const bool click = state.lastActionName == "click_react";

    const float followDrive = follow ? (0.40f + intensity * 0.55f) : 0.0f;
    const float dragDrive = drag ? (0.48f + intensity * 0.65f) : 0.0f;
    const float holdDrive = hold ? (0.45f + intensity * 0.55f) : 0.0f;
    const float clickDrive = click ? (0.35f + intensity * 0.45f) : 0.0f;
    const float scrollDrive = scroll ? std::abs(signedIntensity) : 0.0f;

    profile.bodyArchPx += followDrive * 2.2f + dragDrive * 3.6f;
    profile.bodyArchPx -= holdDrive * 1.8f;
    profile.bodyArchPx -= clickDrive * 1.2f;

    profile.headLiftPx += followDrive * 2.5f + dragDrive * 3.4f;
    profile.headLiftPx -= holdDrive * 1.6f;
    profile.headLiftPx -= clickDrive * 1.2f;
    profile.headForwardPx += facingSign * (followDrive * 2.5f + dragDrive * 4.6f + scrollDrive * 1.5f);
    profile.headForwardPx += motion.reactive.scrollDirection * scrollDrive * 1.0f;

    profile.chestLiftPx += followDrive * 1.2f + dragDrive * 0.8f;
    profile.chestLiftPx -= holdDrive * 1.5f;
    profile.chestLiftPx -= clickDrive * 0.9f;
    profile.chestWidthScale += followDrive * 0.04f + dragDrive * 0.08f - holdDrive * 0.06f + clickDrive * 0.02f;

    profile.tailRootLiftPx += followDrive * 1.0f + dragDrive * 2.0f + scrollDrive * 0.8f;
    profile.tailRootLiftPx += motion.tailLiftPx * 0.10f;
    profile.tailCurlPx += facingSign * (dragDrive * 1.6f + followDrive * 0.9f);
    profile.tailCurlPx += motion.reactive.scrollDirection * scrollDrive * 1.0f;

    profile.shoulderPatchScale += followDrive * 0.06f + dragDrive * 0.12f + clickDrive * 0.03f;
    profile.hipPatchScale += dragDrive * 0.10f + holdDrive * 0.06f;
    profile.frontDepthBiasPx += facingSign * (followDrive * 1.2f + dragDrive * 2.4f) + posture.silhouetteBiasPx * 0.35f;
    profile.rearDepthBiasPx -= facingSign * (followDrive * 0.8f + dragDrive * 1.6f);
    profile.earRootScale += followDrive * 0.06f + dragDrive * 0.04f + scrollDrive * 0.03f - holdDrive * 0.04f;

    profile.chestWidthScale = std::clamp(profile.chestWidthScale, 0.88f, 1.18f);
    profile.shoulderPatchScale = std::clamp(profile.shoulderPatchScale, 0.90f, 1.22f);
    profile.hipPatchScale = std::clamp(profile.hipPatchScale, 0.92f, 1.20f);
    profile.earRootScale = std::clamp(profile.earRootScale, 0.88f, 1.16f);
    return profile;
}

} // namespace mousefx::windows
