#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseSolveProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseSolveState(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& constraintProfile) {
    if (constraintProfile.constraintState == "pose_constraint_bound") {
        return "pose_solve_bound";
    }
    if (constraintProfile.constraintState == "pose_constraint_unbound") {
        return "pose_solve_unbound";
    }
    if (constraintProfile.constraintState == "pose_constraint_runtime_only") {
        return "pose_solve_runtime_only";
    }
    if (constraintProfile.constraintState == "pose_constraint_stub_ready") {
        return "pose_solve_stub_ready";
    }
    if (constraintProfile.constraintState == "pose_constraint_scaffold") {
        return "pose_solve_scaffold";
    }
    return "preview_only";
}

const char* ResolveAssetNodePath(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "/pet/body/root";
    }
    if (logicalNode == "head") {
        return "/pet/body/head";
    }
    if (logicalNode == "appendage") {
        return "/pet/body/appendage";
    }
    if (logicalNode == "overlay") {
        return "/pet/fx/overlay";
    }
    if (logicalNode == "grounding") {
        return "/pet/fx/grounding";
    }
    return "/pet/unknown";
}

Win32MouseCompanionRealRendererAssetNodePoseSolveEntry BuildSolveEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry& constraintEntry) {
    Win32MouseCompanionRealRendererAssetNodePoseSolveEntry entry{};
    entry.logicalNode = constraintEntry.logicalNode;
    entry.constraintName = constraintEntry.constraintName;
    entry.assetNodePath = ResolveAssetNodePath(constraintEntry.logicalNode);
    entry.solvedPoseWeight = constraintEntry.constraintStrength;
    entry.solvedPoseX = constraintEntry.biasX * 0.8f;
    entry.solvedPoseY = constraintEntry.biasY * 0.8f;
    entry.solvedPoseScale = 1.0f + (constraintEntry.biasScale - 1.0f) * 0.85f;
    entry.solvedPoseTiltDeg = constraintEntry.biasTiltDeg * 0.85f;
    entry.resolved = constraintEntry.resolved && entry.solvedPoseWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& profile) {
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

std::string BuildPathBrief(const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& profile) {
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

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.1f,%.1f,%.2f,%.1f)|head:(%.2f,%.1f,%.1f,%.2f,%.1f)|appendage:(%.2f,%.1f,%.1f,%.2f,%.1f)|overlay:(%.2f,%.1f,%.1f,%.2f,%.1f)|grounding:(%.2f,%.1f,%.1f,%.2f,%.1f)",
        profile.bodyEntry.solvedPoseWeight,
        profile.bodyEntry.solvedPoseX,
        profile.bodyEntry.solvedPoseY,
        profile.bodyEntry.solvedPoseScale,
        profile.bodyEntry.solvedPoseTiltDeg,
        profile.headEntry.solvedPoseWeight,
        profile.headEntry.solvedPoseX,
        profile.headEntry.solvedPoseY,
        profile.headEntry.solvedPoseScale,
        profile.headEntry.solvedPoseTiltDeg,
        profile.appendageEntry.solvedPoseWeight,
        profile.appendageEntry.solvedPoseX,
        profile.appendageEntry.solvedPoseY,
        profile.appendageEntry.solvedPoseScale,
        profile.appendageEntry.solvedPoseTiltDeg,
        profile.overlayEntry.solvedPoseWeight,
        profile.overlayEntry.solvedPoseX,
        profile.overlayEntry.solvedPoseY,
        profile.overlayEntry.solvedPoseScale,
        profile.overlayEntry.solvedPoseTiltDeg,
        profile.groundingEntry.solvedPoseWeight,
        profile.groundingEntry.solvedPoseX,
        profile.groundingEntry.solvedPoseY,
        profile.groundingEntry.solvedPoseScale,
        profile.groundingEntry.solvedPoseTiltDeg);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseSolveProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& constraintProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseSolveProfile profile{};
    profile.solveState = ResolvePoseSolveState(constraintProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildSolveEntry(constraintProfile.bodyEntry);
    profile.headEntry = BuildSolveEntry(constraintProfile.headEntry);
    profile.appendageEntry = BuildSolveEntry(constraintProfile.appendageEntry);
    profile.overlayEntry = BuildSolveEntry(constraintProfile.overlayEntry);
    profile.groundingEntry = BuildSolveEntry(constraintProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.solveState, profile.entryCount, profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto& body = profile.bodyEntry;
    const auto& head = profile.headEntry;
    const auto& appendage = profile.appendageEntry;
    const auto& overlay = profile.overlayEntry;
    const auto& grounding = profile.groundingEntry;

    scene.bodyAnchor.X += body.solvedPoseX;
    scene.bodyAnchor.Y += body.solvedPoseY;
    scene.headAnchor.X += head.solvedPoseX;
    scene.headAnchor.Y += head.solvedPoseY;
    scene.appendageAnchor.X += appendage.solvedPoseX;
    scene.appendageAnchor.Y += appendage.solvedPoseY;
    scene.overlayAnchor.X += overlay.solvedPoseX;
    scene.overlayAnchor.Y += overlay.solvedPoseY;
    scene.groundingAnchor.X += grounding.solvedPoseX;
    scene.groundingAnchor.Y += grounding.solvedPoseY;
    scene.bodyAnchorScale *= body.solvedPoseScale;
    scene.headAnchorScale *= head.solvedPoseScale;
    scene.appendageAnchorScale *= appendage.solvedPoseScale;
    scene.overlayAnchorScale *= overlay.solvedPoseScale;
    scene.groundingAnchorScale *= grounding.solvedPoseScale;
    scene.bodyTiltDeg += body.solvedPoseTiltDeg + head.solvedPoseTiltDeg * 0.2f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + head.solvedPoseWeight * 2.5f,
        0.0f,
        255.0f);
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + overlay.solvedPoseWeight * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
