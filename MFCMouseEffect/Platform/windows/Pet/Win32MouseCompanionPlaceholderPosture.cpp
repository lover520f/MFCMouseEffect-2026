#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPosture.h"

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

Win32MouseCompanionPlaceholderPosture BuildWin32MouseCompanionPlaceholderPosture(
    const Win32MouseCompanionVisualState& state,
    const Win32MouseCompanionPlaceholderMotion& motion,
    float facingSign) {
    Win32MouseCompanionPlaceholderPosture posture{};

    const float intensity = ClampUnit(state.lastActionIntensity);
    const bool follow = state.lastActionName == "follow";
    const bool drag = state.lastActionName == "drag";
    const bool hold = state.lastActionName == "hold_react";
    const bool scroll = state.lastActionName == "scroll_react";
    const bool click = state.lastActionName == "click_react";
    const float followDrive = follow ? (0.45f + intensity * 0.55f) : 0.0f;
    const float dragDrive = drag ? (0.55f + intensity * 0.60f) : 0.0f;
    const float holdDrive = hold ? (0.50f + intensity * 0.50f) : 0.0f;
    const float scrollDrive = scroll ? std::abs(ClampSigned(state.lastActionIntensity)) : 0.0f;
    const float clickDrive = click ? (0.35f + intensity * 0.40f) : 0.0f;

    posture.bodyCenterYOffset += followDrive * 2.0f + dragDrive * 3.5f + holdDrive * 5.0f;
    posture.bodyCenterYOffset -= clickDrive * 1.8f;
    posture.headAnchorYOffset -= followDrive * 3.0f + dragDrive * 4.0f;
    posture.headAnchorYOffset += holdDrive * 2.0f;

    posture.bodyForwardBiasPx =
        facingSign * (followDrive * 4.5f + dragDrive * 8.0f + scrollDrive * 2.0f);
    posture.headForwardBiasPx =
        facingSign * (followDrive * 7.0f + dragDrive * 10.0f + scrollDrive * 2.5f) +
        motion.reactive.dragTension * facingSign * 2.0f;

    posture.bodyWidthScale += dragDrive * 0.12f + holdDrive * 0.08f + clickDrive * 0.04f;
    posture.bodyWidthScale -= followDrive * 0.03f;
    posture.bodyHeightScale += followDrive * 0.05f - dragDrive * 0.06f - holdDrive * 0.12f - clickDrive * 0.05f;
    posture.headWidthScale += followDrive * 0.04f + dragDrive * 0.03f;
    posture.headHeightScale += followDrive * 0.02f - holdDrive * 0.08f - clickDrive * 0.04f;

    posture.foreStanceSpreadPx += followDrive * 4.0f + dragDrive * 8.5f + scrollDrive * 1.5f;
    posture.rearStanceSpreadPx += followDrive * 2.5f + dragDrive * 5.5f;
    posture.pawYOffset -= followDrive * 1.0f;
    posture.pawYOffset -= dragDrive * 2.0f;
    posture.pawYOffset += holdDrive * 1.5f;

    posture.earBaseSpreadPx += followDrive * 2.5f + dragDrive * 3.5f + scrollDrive * 1.5f;
    posture.earHeightBoostPx += followDrive * 4.0f + dragDrive * 2.0f + scrollDrive * 1.5f;
    posture.earHeightBoostPx -= holdDrive * 2.5f;

    posture.shadowOffsetPx += dragDrive * 2.0f + holdDrive * 1.2f;
    posture.silhouetteBiasPx =
        facingSign * (followDrive * 1.5f + dragDrive * 2.5f) +
        motion.reactive.scrollDirection * scrollDrive * 0.9f;

    posture.bodyWidthScale = std::clamp(posture.bodyWidthScale, 0.86f, 1.28f);
    posture.bodyHeightScale = std::clamp(posture.bodyHeightScale, 0.72f, 1.18f);
    posture.headWidthScale = std::clamp(posture.headWidthScale, 0.90f, 1.18f);
    posture.headHeightScale = std::clamp(posture.headHeightScale, 0.74f, 1.12f);
    return posture;
}

} // namespace mousefx::windows
