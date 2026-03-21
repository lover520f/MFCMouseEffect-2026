#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionVisualRuntime.h"

#include "MouseFx/Utils/TimeUtils.h"

#include <filesystem>

namespace mousefx::windows {
namespace {

constexpr int kDefaultFacingDirection = 1;
constexpr float kFacingMomentumClampPx = 18.0f;
constexpr float kFacingMomentumDecay = 0.45f;
constexpr float kFacingFlipThresholdPx = 8.0f;

std::string ResolveActionName(int actionCode) {
    switch (actionCode) {
    case 1:
        return "follow";
    case 2:
        return "click_react";
    case 3:
        return "drag";
    case 4:
        return "hold_react";
    case 5:
        return "scroll_react";
    default:
        return "idle";
    }
}

bool IsScrollLikeAction(const std::string& actionName) {
    return actionName == "scroll_react";
}

bool IsHoldLikeAction(const std::string& actionName) {
    return actionName == "hold_react";
}

bool IsClickLikeAction(const std::string& actionName) {
    return actionName == "click_react" || actionName == "drag";
}

void UpdateReactiveActionTelemetry(
    Win32MouseCompanionVisualState* state,
    const std::string& actionName,
    float intensity) {
    if (!state) {
        return;
    }

    const uint64_t nowMs = NowMs();
    const float clampedIntensity = std::clamp(intensity, -1.0f, 1.0f);
    const float absIntensity = std::abs(clampedIntensity);
    const bool actionChanged = state->lastReactiveActionName != actionName;
    const bool intensitySpike =
        absIntensity > std::abs(state->lastReactiveActionIntensity) + 0.18f;

    if (IsClickLikeAction(actionName) && (actionChanged || intensitySpike || absIntensity >= 0.80f)) {
        state->lastClickTriggerTickMs = nowMs;
    }

    if (IsHoldLikeAction(actionName) && (actionChanged || intensitySpike || state->lastHoldTriggerTickMs == 0)) {
        state->lastHoldTriggerTickMs = nowMs;
    }

    if (IsScrollLikeAction(actionName)) {
        const float previousSign = (state->lastScrollSignedIntensity > 0.0f) ? 1.0f :
            (state->lastScrollSignedIntensity < 0.0f ? -1.0f : 0.0f);
        const float nextSign = (clampedIntensity > 0.0f) ? 1.0f :
            (clampedIntensity < 0.0f ? -1.0f : 0.0f);
        if (actionChanged || intensitySpike || previousSign != nextSign || absIntensity >= 0.66f) {
            state->lastScrollTriggerTickMs = nowMs;
        }
        state->lastScrollSignedIntensity = clampedIntensity;
    }

    state->lastReactiveActionName = actionName;
    state->lastReactiveActionIntensity = clampedIntensity;
}

void UpdateFacingDirection(Win32MouseCompanionVisualState* state, const ScreenPoint& nextPoint) {
    if (!state) {
        return;
    }
    if (!state->hasLastPoint) {
        return;
    }
    const float dx = static_cast<float>(nextPoint.x - state->lastPoint.x);
    state->facingMomentumPx =
        std::clamp(state->facingMomentumPx * kFacingMomentumDecay + dx,
                   -kFacingMomentumClampPx,
                   kFacingMomentumClampPx);
    if (state->facingMomentumPx >= kFacingFlipThresholdPx) {
        state->facingDirection = 1;
    } else if (state->facingMomentumPx <= -kFacingFlipThresholdPx) {
        state->facingDirection = -1;
    }
}

} // namespace

void ResetWin32MouseCompanionVisualState(
    Win32MouseCompanionVisualState* state,
    const MouseCompanionPetRuntimeConfig& config,
    bool active) {
    if (!state) {
        return;
    }
    *state = {};
    state->config = config;
    state->active = active;
    state->visible = false;
    state->hasLastPoint = false;
    state->facingDirection = kDefaultFacingDirection;
    state->facingMomentumPx = 0.0f;
}

void ApplyWin32MouseCompanionVisualConfig(
    Win32MouseCompanionVisualState* state,
    const MouseCompanionPetRuntimeConfig& config) {
    if (!state) {
        return;
    }
    state->config = config;
    state->active = config.enabled;
}

bool ApplyWin32MouseCompanionModelAssetState(
    Win32MouseCompanionVisualState* state,
    const std::string& modelPath) {
    if (!state) {
        return false;
    }
    state->lastModelPath = modelPath;
    state->modelAssetAvailable =
        !modelPath.empty() && std::filesystem::exists(std::filesystem::path(modelPath));
    return state->modelAssetAvailable;
}

void ApplyWin32MouseCompanionAppearanceState(
    Win32MouseCompanionVisualState* state,
    Win32MouseCompanionAppearanceProfile profile,
    bool loaded) {
    if (!state) {
        return;
    }
    state->appearanceProfile = loaded ? std::move(profile) : Win32MouseCompanionAppearanceProfile{};
}

void ApplyWin32MouseCompanionPoseBindingState(
    Win32MouseCompanionVisualState* state,
    bool configured) {
    if (!state) {
        return;
    }
    state->poseBindingConfigured = configured && state->active;
}

void ApplyWin32MouseCompanionFollowPoint(
    Win32MouseCompanionVisualState* state,
    const ScreenPoint& pt) {
    if (!state || !state->active) {
        return;
    }
    UpdateFacingDirection(state, pt);
    state->lastPoint = pt;
    state->hasLastPoint = true;
}

void ApplyWin32MouseCompanionHostUpdate(
    Win32MouseCompanionVisualState* state,
    const PetVisualHostUpdate& update) {
    if (!state || !state->active) {
        return;
    }
    UpdateFacingDirection(state, update.pt);
    state->lastPoint = update.pt;
    state->hasLastPoint = true;
    state->lastActionCode = update.actionCode;
    state->lastActionIntensity = update.actionIntensity;
    state->lastHeadTintAmount = update.headTintAmount;
    state->lastActionName = ResolveActionName(update.actionCode);
    UpdateReactiveActionTelemetry(state, state->lastActionName, update.actionIntensity);
}

void ApplyWin32MouseCompanionPoseFrame(
    Win32MouseCompanionVisualState* state,
    const MouseCompanionPetPoseFrame& poseFrame) {
    if (!state || !state->active) {
        return;
    }
    state->lastPoseSampleTickMs = poseFrame.sampleTickMs;
    state->latestPoseFrame = poseFrame;
    state->lastActionName = poseFrame.actionName;
    state->lastActionIntensity = poseFrame.actionIntensity;
    state->lastHeadTintAmount = poseFrame.headTintAmount;
    state->poseFrameAvailable = !poseFrame.samples.empty();
    UpdateReactiveActionTelemetry(state, state->lastActionName, poseFrame.actionIntensity);
}

} // namespace mousefx::windows
