#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeLiftProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeAttachProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveLiftWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& nodeAttachProfile,
    const Win32MouseCompanionRealRendererPoseAdapterProfile& poseAdapterProfile) {
    return std::clamp(
        nodeAttachProfile.attachWeight * 0.68f + poseAdapterProfile.influence * 0.32f,
        0.0f,
        1.0f);
}

std::string ResolveLiftState(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& nodeAttachProfile,
    const Win32MouseCompanionRealRendererPoseAdapterProfile& poseAdapterProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeAttachProfile.attachState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseAdapterProfile.influence > 0.0f) {
            return "model_asset_node_lift_bound";
        }
        if (adapterMode == "pose_unbound" && poseAdapterProfile.influence > 0.0f) {
            return "model_asset_node_lift_pose_ready";
        }
        return "model_asset_node_lift_ready";
    }
    return "model_asset_node_lift_partial";
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        state.empty() ? "preview_only" : state.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildLiftBrief(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& nodeAttachProfile,
    const Win32MouseCompanionRealRendererPoseAdapterProfile& poseAdapterProfile,
    const std::string& adapterMode) {
    const bool attachReady = nodeAttachProfile.attachWeight > 0.0f;
    const bool poseReady = poseAdapterProfile.influence > 0.0f;
    return "body:" + std::string(attachReady ? "lifted" : "stub") +
           "|head:" + std::string(attachReady ? "lifted" : "stub") +
           "|appendage:" + std::string(attachReady ? "lifted" : "stub") +
           "|overlay:" + std::string(poseReady ? "lifted" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float liftWeight,
    const Win32MouseCompanionRealRendererPoseAdapterProfile& poseAdapterProfile,
    const std::string& adapterMode) {
    const float bodyWeight = liftWeight * 0.92f;
    const float headWeight = liftWeight * 0.96f;
    const float appendageWeight = liftWeight * 1.02f;
    const float overlayWeight = liftWeight * std::clamp(poseAdapterProfile.readabilityBias, 0.0f, 1.0f);
    const float adapterWeight =
        adapterMode == "pose_bound" ? liftWeight :
        (adapterMode == "pose_unbound" ? liftWeight * 0.88f : liftWeight * 0.72f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|adapter:%.2f",
        bodyWeight,
        headWeight,
        appendageWeight,
        overlayWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetNodeLiftProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& nodeAttachProfile,
    const Win32MouseCompanionRealRendererPoseAdapterProfile& poseAdapterProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeLiftProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (nodeAttachProfile.attachState == "preview_only" ? 0u : 1u) +
        (nodeAttachProfile.attachWeight > 0.0f ? 1u : 0u) +
        (poseAdapterProfile.influence > 0.0f ? 1u : 0u) +
        (poseAdapterProfile.readabilityBias > 0.0f ? 1u : 0u) +
        (adapterMode != "runtime_only" ? 1u : 0u);
    profile.liftWeight = ResolveLiftWeight(nodeAttachProfile, poseAdapterProfile);
    profile.liftState = ResolveLiftState(
        nodeAttachProfile,
        poseAdapterProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.liftState, profile.entryCount, profile.resolvedEntryCount);
    profile.liftBrief = BuildLiftBrief(nodeAttachProfile, poseAdapterProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.liftWeight, poseAdapterProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeLiftProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
        runtime.modelAssetNodeAttachProfile,
        runtime.poseAdapterProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeLiftProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.liftWeight * 0.012f;
    scene.headAnchorScale *= 1.0f + profile.liftWeight * 0.014f;
    scene.appendageAnchorScale *= 1.0f + profile.liftWeight * 0.022f;
    scene.overlayAnchorScale *= 1.0f + profile.liftWeight * 0.015f;
    scene.bodyTiltDeg += profile.liftWeight * 0.60f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.liftWeight * 6.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
