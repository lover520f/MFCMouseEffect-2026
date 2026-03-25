#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseChannelProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseConstraintState(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelProfile& channelProfile) {
    if (channelProfile.channelState == "pose_channel_bound") {
        return "pose_constraint_bound";
    }
    if (channelProfile.channelState == "pose_channel_unbound") {
        return "pose_constraint_unbound";
    }
    if (channelProfile.channelState == "pose_channel_runtime_only") {
        return "pose_constraint_runtime_only";
    }
    if (channelProfile.channelState == "pose_channel_stub_ready") {
        return "pose_constraint_stub_ready";
    }
    if (channelProfile.channelState == "pose_channel_scaffold") {
        return "pose_constraint_scaffold";
    }
    return "preview_only";
}

const char* ResolveConstraintName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "constraint.body.posture";
    }
    if (logicalNode == "head") {
        return "constraint.head.expression";
    }
    if (logicalNode == "appendage") {
        return "constraint.appendage.motion";
    }
    if (logicalNode == "overlay") {
        return "constraint.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "constraint.grounding.shadow";
    }
    return "constraint.unknown";
}

Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry BuildConstraintEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelEntry& channelEntry) {
    Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry entry{};
    entry.logicalNode = channelEntry.logicalNode;
    entry.channelName = channelEntry.channelName;
    entry.constraintName = ResolveConstraintName(channelEntry.logicalNode);
    entry.constraintStrength = channelEntry.channelWeight;
    entry.biasScale = 1.0f + channelEntry.channelWeight * 0.01f;
    if (channelEntry.logicalNode == "body") {
        entry.biasY = -channelEntry.channelWeight * 0.8f;
        entry.biasTiltDeg = channelEntry.channelWeight * 0.9f;
    } else if (channelEntry.logicalNode == "head") {
        entry.biasY = -channelEntry.channelWeight * 1.0f;
        entry.biasScale = 1.0f + channelEntry.channelWeight * 0.013f;
        entry.biasTiltDeg = channelEntry.channelWeight * 1.1f;
    } else if (channelEntry.logicalNode == "appendage") {
        entry.biasX = channelEntry.channelWeight * 0.7f;
        entry.biasScale = 1.0f + channelEntry.channelWeight * 0.016f;
        entry.biasTiltDeg = channelEntry.channelWeight * 0.8f;
    } else if (channelEntry.logicalNode == "overlay") {
        entry.biasY = -channelEntry.channelWeight * 0.5f;
        entry.biasScale = 1.0f + channelEntry.channelWeight * 0.015f;
    } else if (channelEntry.logicalNode == "grounding") {
        entry.biasY = channelEntry.channelWeight * 0.4f;
        entry.biasScale = 1.0f + channelEntry.channelWeight * 0.012f;
    }
    entry.resolved = channelEntry.resolved && entry.constraintStrength > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& profile) {
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

std::string BuildConstraintBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& profile) {
    char buffer[448];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.constraintName.c_str(),
        profile.headEntry.constraintName.c_str(),
        profile.appendageEntry.constraintName.c_str(),
        profile.overlayEntry.constraintName.c_str(),
        profile.groundingEntry.constraintName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.1f,%.1f,%.2f,%.1f)|head:(%.2f,%.1f,%.1f,%.2f,%.1f)|appendage:(%.2f,%.1f,%.1f,%.2f,%.1f)|overlay:(%.2f,%.1f,%.1f,%.2f,%.1f)|grounding:(%.2f,%.1f,%.1f,%.2f,%.1f)",
        profile.bodyEntry.constraintStrength,
        profile.bodyEntry.biasX,
        profile.bodyEntry.biasY,
        profile.bodyEntry.biasScale,
        profile.bodyEntry.biasTiltDeg,
        profile.headEntry.constraintStrength,
        profile.headEntry.biasX,
        profile.headEntry.biasY,
        profile.headEntry.biasScale,
        profile.headEntry.biasTiltDeg,
        profile.appendageEntry.constraintStrength,
        profile.appendageEntry.biasX,
        profile.appendageEntry.biasY,
        profile.appendageEntry.biasScale,
        profile.appendageEntry.biasTiltDeg,
        profile.overlayEntry.constraintStrength,
        profile.overlayEntry.biasX,
        profile.overlayEntry.biasY,
        profile.overlayEntry.biasScale,
        profile.overlayEntry.biasTiltDeg,
        profile.groundingEntry.constraintStrength,
        profile.groundingEntry.biasX,
        profile.groundingEntry.biasY,
        profile.groundingEntry.biasScale,
        profile.groundingEntry.biasTiltDeg);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelProfile& channelProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile profile{};
    profile.constraintState = ResolvePoseConstraintState(channelProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildConstraintEntry(channelProfile.bodyEntry);
    profile.headEntry = BuildConstraintEntry(channelProfile.headEntry);
    profile.appendageEntry = BuildConstraintEntry(channelProfile.appendageEntry);
    profile.overlayEntry = BuildConstraintEntry(channelProfile.overlayEntry);
    profile.groundingEntry = BuildConstraintEntry(channelProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(
        profile.constraintState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.constraintBrief = BuildConstraintBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto body = profile.bodyEntry;
    const auto head = profile.headEntry;
    const auto appendage = profile.appendageEntry;
    const auto overlay = profile.overlayEntry;
    const auto grounding = profile.groundingEntry;

    scene.bodyAnchorScale *= body.biasScale;
    scene.headAnchorScale *= head.biasScale;
    scene.appendageAnchorScale *= appendage.biasScale;
    scene.overlayAnchorScale *= overlay.biasScale;
    scene.groundingAnchorScale *= grounding.biasScale;
    scene.bodyTiltDeg += body.biasTiltDeg + appendage.biasTiltDeg * 0.25f;
    scene.bodyAnchor.Y += body.biasY;
    scene.headAnchor.Y += head.biasY;
    scene.appendageAnchor.X += appendage.biasX;
    scene.overlayAnchor.Y += overlay.biasY;
    scene.groundingAnchor.Y += grounding.biasY;
    scene.whiskerStrokeWidth *= 1.0f + head.constraintStrength * 0.02f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha * (1.0f + head.constraintStrength * 0.015f),
        0.0f,
        255.0f);
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + overlay.constraintStrength * 10.0f,
        0.0f,
        255.0f);
    scene.accessoryAlphaScale *= 1.0f + appendage.constraintStrength * 0.018f;
    scene.shadowAlphaScale *= 1.0f + grounding.constraintStrength * 0.015f;
    scene.pedestalAlphaScale *= 1.0f + grounding.constraintStrength * 0.02f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha * (1.0f + overlay.constraintStrength * 0.018f),
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha * (1.0f + appendage.constraintStrength * 0.018f),
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha * (1.0f + overlay.constraintStrength * 0.014f),
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha * (1.0f + body.constraintStrength * 0.012f),
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
