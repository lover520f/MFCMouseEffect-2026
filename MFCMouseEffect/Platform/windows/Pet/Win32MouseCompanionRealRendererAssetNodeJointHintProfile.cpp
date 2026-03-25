#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeJointHintProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseSolveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveJointHintState(
    const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& solveProfile) {
    if (solveProfile.solveState == "pose_solve_bound") {
        return "joint_hint_bound";
    }
    if (solveProfile.solveState == "pose_solve_unbound") {
        return "joint_hint_unbound";
    }
    if (solveProfile.solveState == "pose_solve_runtime_only") {
        return "joint_hint_runtime_only";
    }
    if (solveProfile.solveState == "pose_solve_stub_ready") {
        return "joint_hint_stub_ready";
    }
    if (solveProfile.solveState == "pose_solve_scaffold") {
        return "joint_hint_scaffold";
    }
    return "preview_only";
}

const char* ResolveJointHintName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "joint.body.spine";
    }
    if (logicalNode == "head") {
        return "joint.head.look";
    }
    if (logicalNode == "appendage") {
        return "joint.appendage.reach";
    }
    if (logicalNode == "overlay") {
        return "joint.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "joint.grounding.balance";
    }
    return "joint.unknown";
}

Win32MouseCompanionRealRendererAssetNodeJointHintEntry BuildJointHintEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseSolveEntry& solveEntry) {
    Win32MouseCompanionRealRendererAssetNodeJointHintEntry entry{};
    entry.logicalNode = solveEntry.logicalNode;
    entry.assetNodePath = solveEntry.assetNodePath;
    entry.jointHintName = ResolveJointHintName(solveEntry.logicalNode);
    entry.hintWeight = solveEntry.solvedPoseWeight;
    entry.reachBias = solveEntry.solvedPoseX;
    entry.spreadBias = (solveEntry.solvedPoseScale - 1.0f) * 10.0f;
    entry.tiltBiasDeg = solveEntry.solvedPoseTiltDeg;
    entry.resolved = solveEntry.resolved && entry.hintWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) {
        ++count;
    }
    if (profile.headEntry.resolved) {
        ++count;
    }
    if (profile.appendageEntry.resolved) {
        ++count;
    }
    if (profile.overlayEntry.resolved) {
        ++count;
    }
    if (profile.groundingEntry.resolved) {
        ++count;
    }
    return count;
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

std::string BuildJointHintBrief(const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& profile) {
    char buffer[384];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.jointHintName.c_str(),
        profile.headEntry.jointHintName.c_str(),
        profile.appendageEntry.jointHintName.c_str(),
        profile.overlayEntry.jointHintName.c_str(),
        profile.groundingEntry.jointHintName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.1f,%.1f,%.1f)|head:(%.2f,%.1f,%.1f,%.1f)|appendage:(%.2f,%.1f,%.1f,%.1f)|overlay:(%.2f,%.1f,%.1f,%.1f)|grounding:(%.2f,%.1f,%.1f,%.1f)",
        profile.bodyEntry.hintWeight,
        profile.bodyEntry.reachBias,
        profile.bodyEntry.spreadBias,
        profile.bodyEntry.tiltBiasDeg,
        profile.headEntry.hintWeight,
        profile.headEntry.reachBias,
        profile.headEntry.spreadBias,
        profile.headEntry.tiltBiasDeg,
        profile.appendageEntry.hintWeight,
        profile.appendageEntry.reachBias,
        profile.appendageEntry.spreadBias,
        profile.appendageEntry.tiltBiasDeg,
        profile.overlayEntry.hintWeight,
        profile.overlayEntry.reachBias,
        profile.overlayEntry.spreadBias,
        profile.overlayEntry.tiltBiasDeg,
        profile.groundingEntry.hintWeight,
        profile.groundingEntry.reachBias,
        profile.groundingEntry.spreadBias,
        profile.groundingEntry.tiltBiasDeg);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeJointHintProfile
BuildWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& solveProfile) {
    Win32MouseCompanionRealRendererAssetNodeJointHintProfile profile{};
    profile.hintState = ResolveJointHintState(solveProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildJointHintEntry(solveProfile.bodyEntry);
    profile.headEntry = BuildJointHintEntry(solveProfile.headEntry);
    profile.appendageEntry = BuildJointHintEntry(solveProfile.appendageEntry);
    profile.overlayEntry = BuildJointHintEntry(solveProfile.overlayEntry);
    profile.groundingEntry = BuildJointHintEntry(solveProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.hintState, profile.entryCount, profile.resolvedEntryCount);
    profile.jointHintBrief = BuildJointHintBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
    const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto& body = profile.bodyEntry;
    const auto& head = profile.headEntry;
    const auto& appendage = profile.appendageEntry;
    const auto& overlay = profile.overlayEntry;
    const auto& grounding = profile.groundingEntry;

    scene.bodyTiltDeg += body.tiltBiasDeg * 0.35f + head.tiltBiasDeg * 0.15f;
    scene.whiskerStrokeWidth *= 1.0f + head.spreadBias * 0.002f;
    scene.accessoryStrokeWidth *= 1.0f + appendage.spreadBias * 0.002f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + overlay.hintWeight * 6.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + grounding.hintWeight * 0.01f;
    scene.pedestalAlphaScale *= 1.0f + grounding.hintWeight * 0.012f;
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha * (1.0f + appendage.hintWeight * 0.015f),
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha * (1.0f + body.hintWeight * 0.01f),
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
