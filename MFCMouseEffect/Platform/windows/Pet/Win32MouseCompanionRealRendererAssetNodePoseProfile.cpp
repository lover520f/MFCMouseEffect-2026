#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile) {
    if (worldSpaceProfile.worldSpaceState == "world_space_ready") {
        if (runtime.sceneRuntimeAdapterMode == "pose_bound") {
            return "pose_table_bound";
        }
        if (runtime.sceneRuntimeAdapterMode == "pose_unbound") {
            return "pose_table_unbound";
        }
        return "pose_table_runtime_only";
    }
    if (worldSpaceProfile.worldSpaceState == "world_space_stub_ready") {
        return "pose_table_stub_ready";
    }
    if (worldSpaceProfile.worldSpaceState == "world_space_scaffold") {
        return "pose_table_scaffold";
    }
    return "preview_only";
}

float ResolveNodePoseMultiplier(const std::string& logicalNode) {
    if (logicalNode == "head") {
        return 1.08f;
    }
    if (logicalNode == "appendage") {
        return 1.12f;
    }
    if (logicalNode == "overlay") {
        return 1.04f;
    }
    if (logicalNode == "grounding") {
        return 0.94f;
    }
    return 1.0f;
}

float ResolveNodePoseTilt(
    const std::string& logicalNode,
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene,
    float poseWeight) {
    const float facingSign = scene.facingSign >= 0.0f ? 1.0f : -1.0f;
    const float actionBias = std::clamp(
        runtime.actionIntensity * 0.85f + runtime.reactiveActionIntensity * 0.55f,
        0.0f,
        1.0f);
    if (logicalNode == "head") {
        return scene.bodyTiltDeg * (0.34f + poseWeight * 0.16f) + facingSign * actionBias * 2.6f;
    }
    if (logicalNode == "appendage") {
        return scene.bodyTiltDeg * (0.18f + poseWeight * 0.12f) + facingSign * actionBias * 1.8f;
    }
    if (logicalNode == "overlay") {
        return scene.bodyTiltDeg * 0.12f + facingSign * poseWeight * 1.1f;
    }
    if (logicalNode == "grounding") {
        return scene.bodyTiltDeg * 0.08f;
    }
    return scene.bodyTiltDeg * (0.16f + poseWeight * 0.08f);
}

Win32MouseCompanionRealRendererAssetNodePoseEntry BuildPoseEntry(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& worldSpaceEntry,
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene) {
    Win32MouseCompanionRealRendererAssetNodePoseEntry entry{};
    entry.logicalNode = worldSpaceEntry.logicalNode;
    entry.assetNodePath = worldSpaceEntry.assetNodePath;
    const float baseInfluence = std::clamp(runtime.poseAdapterProfile.influence, 0.0f, 1.0f);
    const float readabilityBias = std::clamp(runtime.poseAdapterProfile.readabilityBias, 0.0f, 1.0f);
    entry.poseWeight = std::clamp(
        worldSpaceEntry.worldWeight * (0.72f + baseInfluence * 0.20f + readabilityBias * 0.08f) *
            ResolveNodePoseMultiplier(worldSpaceEntry.logicalNode),
        0.0f,
        1.6f);
    entry.poseX = worldSpaceEntry.worldX;
    entry.poseY = worldSpaceEntry.worldY;
    entry.poseScale =
        1.0f + (worldSpaceEntry.worldScale - 1.0f) * (0.90f + entry.poseWeight * 0.10f);
    entry.poseTiltDeg = ResolveNodePoseTilt(
        worldSpaceEntry.logicalNode,
        runtime,
        scene,
        entry.poseWeight);
    entry.resolved = worldSpaceEntry.resolved && entry.poseWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& state,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
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

std::string BuildPathBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.assetNodePath.c_str(),
        profile.headEntry.assetNodePath.c_str(),
        profile.appendageEntry.assetNodePath.c_str(),
        profile.overlayEntry.assetNodePath.c_str(),
        profile.groundingEntry.assetNodePath.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& profile) {
    char buffer[384];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.1f,%.1f,%.2f,%.1f)|head:(%.1f,%.1f,%.2f,%.1f)|appendage:(%.1f,%.1f,%.2f,%.1f)|overlay:(%.1f,%.1f,%.2f,%.1f)|grounding:(%.1f,%.1f,%.2f,%.1f)",
        profile.bodyEntry.poseX,
        profile.bodyEntry.poseY,
        profile.bodyEntry.poseScale,
        profile.bodyEntry.poseTiltDeg,
        profile.headEntry.poseX,
        profile.headEntry.poseY,
        profile.headEntry.poseScale,
        profile.headEntry.poseTiltDeg,
        profile.appendageEntry.poseX,
        profile.appendageEntry.poseY,
        profile.appendageEntry.poseScale,
        profile.appendageEntry.poseTiltDeg,
        profile.overlayEntry.poseX,
        profile.overlayEntry.poseY,
        profile.overlayEntry.poseScale,
        profile.overlayEntry.poseTiltDeg,
        profile.groundingEntry.poseX,
        profile.groundingEntry.poseY,
        profile.groundingEntry.poseScale,
        profile.groundingEntry.poseTiltDeg);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene,
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseProfile profile{};
    profile.poseState = ResolvePoseState(runtime, worldSpaceProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildPoseEntry(worldSpaceProfile.bodyEntry, runtime, scene);
    profile.headEntry = BuildPoseEntry(worldSpaceProfile.headEntry, runtime, scene);
    profile.appendageEntry = BuildPoseEntry(worldSpaceProfile.appendageEntry, runtime, scene);
    profile.overlayEntry = BuildPoseEntry(worldSpaceProfile.overlayEntry, runtime, scene);
    profile.groundingEntry = BuildPoseEntry(worldSpaceProfile.groundingEntry, runtime, scene);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.poseState, profile.entryCount, profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodePoseProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const float bodyWeight = profile.bodyEntry.resolved ? profile.bodyEntry.poseWeight : 0.0f;
    const float headWeight = profile.headEntry.resolved ? profile.headEntry.poseWeight : 0.0f;
    const float appendageWeight =
        profile.appendageEntry.resolved ? profile.appendageEntry.poseWeight : 0.0f;
    const float overlayWeight = profile.overlayEntry.resolved ? profile.overlayEntry.poseWeight : 0.0f;
    const float groundingWeight =
        profile.groundingEntry.resolved ? profile.groundingEntry.poseWeight : 0.0f;

    scene.bodyStrokeWidth += bodyWeight * 0.08f;
    scene.headStrokeWidth += headWeight * 0.10f;
    scene.earStrokeWidth += appendageWeight * 0.06f;
    scene.whiskerStrokeWidth += headWeight * 0.04f;
    scene.glowAlpha = std::clamp(
        scene.glowAlpha * (1.0f + headWeight * 0.03f + overlayWeight * 0.02f),
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + groundingWeight * 0.02f;
    scene.pedestalAlphaScale *= 1.0f + groundingWeight * 0.025f;
    scene.accessoryAlphaScale *= 1.0f + appendageWeight * 0.03f;
    scene.accessoryStrokeWidth += appendageWeight * 0.06f;
    scene.actionOverlay.clickRingStrokeWidth += overlayWeight * 0.08f;
    scene.actionOverlay.dragLineStrokeWidth += appendageWeight * 0.05f;
    scene.actionOverlay.scrollArcStrokeWidth += overlayWeight * 0.06f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha * (1.0f + overlayWeight * 0.02f),
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
