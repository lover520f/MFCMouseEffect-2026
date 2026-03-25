#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseChannelProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseChannelState(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& registryProfile) {
    if (registryProfile.registryState == "pose_registry_bound") {
        return "pose_channel_bound";
    }
    if (registryProfile.registryState == "pose_registry_unbound") {
        return "pose_channel_unbound";
    }
    if (registryProfile.registryState == "pose_registry_runtime_only") {
        return "pose_channel_runtime_only";
    }
    if (registryProfile.registryState == "pose_registry_stub_ready") {
        return "pose_channel_stub_ready";
    }
    if (registryProfile.registryState == "pose_registry_scaffold") {
        return "pose_channel_scaffold";
    }
    return "preview_only";
}

const char* ResolvePoseChannelName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "channel.body.posture";
    }
    if (logicalNode == "head") {
        return "channel.head.expression";
    }
    if (logicalNode == "appendage") {
        return "channel.appendage.motion";
    }
    if (logicalNode == "overlay") {
        return "channel.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "channel.grounding.shadow";
    }
    return "channel.unknown";
}

float ResolvePoseChannelWeight(const std::string& logicalNode, float registryWeight) {
    if (logicalNode == "head") {
        return registryWeight * 1.04f;
    }
    if (logicalNode == "appendage") {
        return registryWeight * 1.06f;
    }
    if (logicalNode == "overlay") {
        return registryWeight * 1.02f;
    }
    if (logicalNode == "grounding") {
        return registryWeight * 0.98f;
    }
    return registryWeight;
}

Win32MouseCompanionRealRendererAssetNodePoseChannelEntry BuildPoseChannelEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodePoseChannelEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.poseNodeName = registryEntry.poseNodeName;
    entry.channelName = ResolvePoseChannelName(registryEntry.logicalNode);
    entry.channelWeight =
        ResolvePoseChannelWeight(registryEntry.logicalNode, registryEntry.registryWeight);
    entry.resolved = registryEntry.resolved && entry.channelWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelProfile& profile) {
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

std::string BuildChannelBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelProfile& profile) {
    char buffer[384];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.channelName.c_str(),
        profile.headEntry.channelName.c_str(),
        profile.appendageEntry.channelName.c_str(),
        profile.overlayEntry.channelName.c_str(),
        profile.groundingEntry.channelName.c_str());
    return std::string(buffer);
}

std::string BuildWeightBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyEntry.channelWeight,
        profile.headEntry.channelWeight,
        profile.appendageEntry.channelWeight,
        profile.overlayEntry.channelWeight,
        profile.groundingEntry.channelWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseChannelProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseChannelProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& registryProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseChannelProfile profile{};
    profile.channelState = ResolvePoseChannelState(registryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildPoseChannelEntry(registryProfile.bodyEntry);
    profile.headEntry = BuildPoseChannelEntry(registryProfile.headEntry);
    profile.appendageEntry = BuildPoseChannelEntry(registryProfile.appendageEntry);
    profile.overlayEntry = BuildPoseChannelEntry(registryProfile.overlayEntry);
    profile.groundingEntry = BuildPoseChannelEntry(registryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.channelState, profile.entryCount, profile.resolvedEntryCount);
    profile.channelBrief = BuildChannelBrief(profile);
    profile.weightBrief = BuildWeightBrief(profile);
    return profile;
}

} // namespace mousefx::windows
