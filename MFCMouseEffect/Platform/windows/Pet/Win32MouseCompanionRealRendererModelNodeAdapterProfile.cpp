#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeAdapterProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

float AveragePoseAxis(
    const MouseCompanionPetPoseSample* first,
    const MouseCompanionPetPoseSample* second,
    size_t axis) {
    float sum = 0.0f;
    float count = 0.0f;
    if (first) {
        sum += first->position[axis];
        count += 1.0f;
    }
    if (second) {
        sum += second->position[axis];
        count += 1.0f;
    }
    return count > 0.0f ? sum / count : 0.0f;
}

float ResolveModelNodeInfluence(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const float poseInfluence = runtime.poseAdapterProfile.influence;
    const std::string& seamState = runtime.modelSceneAdapterProfile.seamState;
    if (seamState == "pose_bound_preview_ready") {
        return poseInfluence;
    }
    if (seamState == "pose_stub_ready") {
        return poseInfluence * 0.72f;
    }
    if (seamState == "asset_stub_ready") {
        return poseInfluence * 0.35f;
    }
    return 0.0f;
}

float ResolveChannelInfluence(
    float baseInfluence,
    const std::string& seamState,
    float assetScale,
    float poseStubScale,
    float poseBoundScale) {
    if (seamState == "pose_bound_preview_ready") {
        return baseInfluence * poseBoundScale;
    }
    if (seamState == "pose_stub_ready") {
        return baseInfluence * poseStubScale;
    }
    if (seamState == "asset_stub_ready") {
        return baseInfluence * assetScale;
    }
    return 0.0f;
}

std::string BuildModelNodeAdapterBrief(
    const std::string& seamState,
    float influence) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%.2f",
        seamState.empty() ? "preview_only" : seamState.c_str(),
        influence);
    return std::string(buffer);
}

std::string BuildModelNodeChannelBrief(
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|face:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyChannel.influence,
        profile.faceChannel.influence,
        profile.appendageChannel.influence,
        profile.overlayChannel.influence,
        profile.groundingChannel.influence);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelNodeAdapterProfile
BuildWin32MouseCompanionRealRendererModelNodeAdapterProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererModelNodeAdapterProfile profile{};
    profile.influence = ResolveModelNodeInfluence(runtime);

    const float handReachX =
        AveragePoseAxis(runtime.leftHandPose, runtime.rightHandPose, 0);
    const float handLiftY =
        AveragePoseAxis(runtime.leftHandPose, runtime.rightHandPose, 1);
    const float legReachX =
        AveragePoseAxis(runtime.leftLegPose, runtime.rightLegPose, 0);
    const float legLiftY =
        AveragePoseAxis(runtime.leftLegPose, runtime.rightLegPose, 1);

    profile.bodyChannel.influence = ResolveChannelInfluence(
        profile.influence,
        runtime.modelSceneAdapterProfile.seamState,
        0.35f,
        0.74f,
        1.00f);
    profile.faceChannel.influence = ResolveChannelInfluence(
        profile.influence,
        runtime.modelSceneAdapterProfile.seamState,
        0.28f,
        0.78f,
        1.06f);
    profile.appendageChannel.influence = ResolveChannelInfluence(
        profile.influence,
        runtime.modelSceneAdapterProfile.seamState,
        0.42f,
        0.82f,
        1.08f);
    profile.overlayChannel.influence = ResolveChannelInfluence(
        profile.influence,
        runtime.modelSceneAdapterProfile.seamState,
        0.24f,
        0.68f,
        0.96f);
    profile.groundingChannel.influence = ResolveChannelInfluence(
        profile.influence,
        runtime.modelSceneAdapterProfile.seamState,
        0.30f,
        0.70f,
        0.92f);

    profile.bodyChannel.offsetX =
        handReachX * 0.045f + legReachX * 0.030f;
    profile.bodyChannel.offsetY =
        -handLiftY * 0.032f - legLiftY * 0.016f;
    profile.faceChannel.offsetX = handReachX * 0.022f;
    profile.faceChannel.offsetY = -handLiftY * 0.030f;
    profile.appendageChannel.offsetX =
        handReachX * 0.020f + legReachX * 0.015f;
    profile.appendageChannel.offsetY =
        -handLiftY * 0.018f - legLiftY * 0.010f;
    profile.overlayChannel.offsetX =
        handReachX * 0.030f + legReachX * 0.018f;
    profile.overlayChannel.offsetY = -handLiftY * 0.030f;
    profile.groundingChannel.offsetX = legReachX * 0.022f;
    profile.groundingChannel.offsetY = -legLiftY * 0.018f;

    profile.centerOffsetX =
        profile.bodyChannel.offsetX * profile.bodyChannel.influence;
    profile.centerOffsetY =
        profile.bodyChannel.offsetY * profile.bodyChannel.influence;
    profile.faceOffsetX =
        profile.faceChannel.offsetX * profile.faceChannel.influence;
    profile.faceOffsetY =
        profile.faceChannel.offsetY * profile.faceChannel.influence;
    profile.overlayOffsetX =
        profile.overlayChannel.offsetX * profile.overlayChannel.influence;
    profile.overlayOffsetY =
        profile.overlayChannel.offsetY * profile.overlayChannel.influence;
    profile.adornmentOffsetX =
        profile.appendageChannel.offsetX * profile.appendageChannel.influence;
    profile.adornmentOffsetY =
        profile.appendageChannel.offsetY * profile.appendageChannel.influence;
    profile.groundingOffsetX =
        profile.groundingChannel.offsetX * profile.groundingChannel.influence;
    profile.groundingOffsetY =
        profile.groundingChannel.offsetY * profile.groundingChannel.influence;
    profile.whiskerBias = handReachX * 0.16f * profile.faceChannel.influence;
    profile.blushLift = (-handLiftY) * 1.2f * profile.faceChannel.influence;
    profile.brief = BuildModelNodeAdapterBrief(
        runtime.modelSceneAdapterProfile.seamState,
        profile.influence);
    profile.channelBrief = BuildModelNodeChannelBrief(profile);
    return profile;
}

} // namespace mousefx::windows
