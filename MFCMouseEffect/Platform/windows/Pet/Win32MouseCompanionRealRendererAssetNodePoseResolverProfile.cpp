#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseResolverProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseProfile.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseResolverState(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& poseProfile) {
    if (poseProfile.poseState == "pose_table_bound") {
        return "pose_resolver_bound";
    }
    if (poseProfile.poseState == "pose_table_unbound") {
        return "pose_resolver_unbound";
    }
    if (poseProfile.poseState == "pose_table_runtime_only") {
        return "pose_resolver_runtime_only";
    }
    if (poseProfile.poseState == "pose_table_stub_ready") {
        return "pose_resolver_stub_ready";
    }
    if (poseProfile.poseState == "pose_table_scaffold") {
        return "pose_resolver_scaffold";
    }
    return "preview_only";
}

const char* ResolvePoseKind(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "body_pose";
    }
    if (logicalNode == "head") {
        return "head_pose";
    }
    if (logicalNode == "appendage") {
        return "appendage_pose";
    }
    if (logicalNode == "overlay") {
        return "overlay_pose";
    }
    if (logicalNode == "grounding") {
        return "grounding_pose";
    }
    return "unknown_pose";
}

float ResolvePoseResolverWeight(const std::string& logicalNode, float poseWeight) {
    if (logicalNode == "head") {
        return poseWeight * 1.04f;
    }
    if (logicalNode == "appendage") {
        return poseWeight * 1.06f;
    }
    if (logicalNode == "overlay") {
        return poseWeight * 0.98f;
    }
    if (logicalNode == "grounding") {
        return poseWeight * 0.95f;
    }
    return poseWeight;
}

Win32MouseCompanionRealRendererAssetNodePoseResolverEntry BuildPoseResolverEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseEntry& poseEntry) {
    Win32MouseCompanionRealRendererAssetNodePoseResolverEntry entry{};
    entry.logicalNode = poseEntry.logicalNode;
    entry.assetNodePath = poseEntry.assetNodePath;
    entry.poseKind = ResolvePoseKind(poseEntry.logicalNode);
    entry.resolvedPoseWeight =
        ResolvePoseResolverWeight(poseEntry.logicalNode, poseEntry.poseWeight);
    entry.resolvedPoseX = poseEntry.poseX * (0.82f + entry.resolvedPoseWeight * 0.14f);
    entry.resolvedPoseY = poseEntry.poseY * (0.84f + entry.resolvedPoseWeight * 0.12f);
    entry.resolvedPoseScale =
        1.0f + (poseEntry.poseScale - 1.0f) * (0.88f + entry.resolvedPoseWeight * 0.12f);
    entry.resolvedPoseTiltDeg =
        poseEntry.poseTiltDeg * (0.76f + entry.resolvedPoseWeight * 0.18f);
    entry.resolved =
        poseEntry.resolved && entry.resolvedPoseWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodePoseResolverProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
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

std::string BuildPathBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseResolverProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodePoseResolverProfile& profile) {
    char buffer[384];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.1f,%.1f,%.2f,%.1f)|head:(%.1f,%.1f,%.2f,%.1f)|appendage:(%.1f,%.1f,%.2f,%.1f)|overlay:(%.1f,%.1f,%.2f,%.1f)|grounding:(%.1f,%.1f,%.2f,%.1f)",
        profile.bodyEntry.resolvedPoseX,
        profile.bodyEntry.resolvedPoseY,
        profile.bodyEntry.resolvedPoseScale,
        profile.bodyEntry.resolvedPoseTiltDeg,
        profile.headEntry.resolvedPoseX,
        profile.headEntry.resolvedPoseY,
        profile.headEntry.resolvedPoseScale,
        profile.headEntry.resolvedPoseTiltDeg,
        profile.appendageEntry.resolvedPoseX,
        profile.appendageEntry.resolvedPoseY,
        profile.appendageEntry.resolvedPoseScale,
        profile.appendageEntry.resolvedPoseTiltDeg,
        profile.overlayEntry.resolvedPoseX,
        profile.overlayEntry.resolvedPoseY,
        profile.overlayEntry.resolvedPoseScale,
        profile.overlayEntry.resolvedPoseTiltDeg,
        profile.groundingEntry.resolvedPoseX,
        profile.groundingEntry.resolvedPoseY,
        profile.groundingEntry.resolvedPoseScale,
        profile.groundingEntry.resolvedPoseTiltDeg);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseResolverProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseResolverProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& poseProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseResolverProfile profile{};
    profile.resolverState = ResolvePoseResolverState(poseProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildPoseResolverEntry(poseProfile.bodyEntry);
    profile.headEntry = BuildPoseResolverEntry(poseProfile.headEntry);
    profile.appendageEntry = BuildPoseResolverEntry(poseProfile.appendageEntry);
    profile.overlayEntry = BuildPoseResolverEntry(poseProfile.overlayEntry);
    profile.groundingEntry = BuildPoseResolverEntry(poseProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.resolverState, profile.entryCount, profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

} // namespace mousefx::windows
