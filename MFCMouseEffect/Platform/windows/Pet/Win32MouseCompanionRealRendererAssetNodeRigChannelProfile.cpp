#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigChannelProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRigChannelState(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& controlRigHintProfile) {
    if (controlRigHintProfile.hintState == "control_rig_hint_bound") {
        return "rig_channel_bound";
    }
    if (controlRigHintProfile.hintState == "control_rig_hint_unbound") {
        return "rig_channel_unbound";
    }
    if (controlRigHintProfile.hintState == "control_rig_hint_runtime_only") {
        return "rig_channel_runtime_only";
    }
    if (controlRigHintProfile.hintState == "control_rig_hint_stub_ready") {
        return "rig_channel_stub_ready";
    }
    if (controlRigHintProfile.hintState == "control_rig_hint_scaffold") {
        return "rig_channel_scaffold";
    }
    return "preview_only";
}

const char* ResolveRigChannelName(const std::string& logicalNode) {
    if (logicalNode == "body") return "rig.channel.body.spine";
    if (logicalNode == "head") return "rig.channel.head.look";
    if (logicalNode == "appendage") return "rig.channel.appendage.reach";
    if (logicalNode == "overlay") return "rig.channel.overlay.fx";
    if (logicalNode == "grounding") return "rig.channel.grounding.balance";
    return "rig.channel.unknown";
}

Win32MouseCompanionRealRendererAssetNodeRigChannelEntry BuildRigChannelEntry(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry& hintEntry) {
    Win32MouseCompanionRealRendererAssetNodeRigChannelEntry entry{};
    entry.logicalNode = hintEntry.logicalNode;
    entry.rigHintName = hintEntry.rigHintName;
    entry.rigChannelName = ResolveRigChannelName(hintEntry.logicalNode);
    entry.channelWeight = hintEntry.hintWeight;
    entry.amplitudeBias = hintEntry.driveBias * 1.25f;
    entry.responseBias = hintEntry.dampingBias * 1.4f;
    entry.resolved = hintEntry.resolved && entry.channelWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) ++count;
    if (profile.headEntry.resolved) ++count;
    if (profile.appendageEntry.resolved) ++count;
    if (profile.overlayEntry.resolved) ++count;
    if (profile.groundingEntry.resolved) ++count;
    return count;
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), "%s/%u/%u",
                  state.empty() ? "preview_only" : state.c_str(),
                  entryCount,
                  resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildChannelBrief(const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& profile) {
    char buffer[448];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.rigChannelName.c_str(),
                  profile.headEntry.rigChannelName.c_str(),
                  profile.appendageEntry.rigChannelName.c_str(),
                  profile.overlayEntry.rigChannelName.c_str(),
                  profile.groundingEntry.rigChannelName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.channelWeight,
        profile.bodyEntry.amplitudeBias,
        profile.bodyEntry.responseBias,
        profile.headEntry.channelWeight,
        profile.headEntry.amplitudeBias,
        profile.headEntry.responseBias,
        profile.appendageEntry.channelWeight,
        profile.appendageEntry.amplitudeBias,
        profile.appendageEntry.responseBias,
        profile.overlayEntry.channelWeight,
        profile.overlayEntry.amplitudeBias,
        profile.overlayEntry.responseBias,
        profile.groundingEntry.channelWeight,
        profile.groundingEntry.amplitudeBias,
        profile.groundingEntry.responseBias);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeRigChannelProfile
BuildWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& controlRigHintProfile) {
    Win32MouseCompanionRealRendererAssetNodeRigChannelProfile profile{};
    profile.channelState = ResolveRigChannelState(controlRigHintProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRigChannelEntry(controlRigHintProfile.bodyEntry);
    profile.headEntry = BuildRigChannelEntry(controlRigHintProfile.headEntry);
    profile.appendageEntry = BuildRigChannelEntry(controlRigHintProfile.appendageEntry);
    profile.overlayEntry = BuildRigChannelEntry(controlRigHintProfile.overlayEntry);
    profile.groundingEntry = BuildRigChannelEntry(controlRigHintProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.channelState, profile.entryCount, profile.resolvedEntryCount);
    profile.channelBrief = BuildChannelBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.amplitudeBias * 0.12f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.amplitudeBias * 0.14f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.amplitudeBias * 0.18f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.amplitudeBias * 0.10f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.amplitudeBias * 0.09f;
    scene.bodyTiltDeg += profile.bodyEntry.responseBias * 1.8f;
    scene.actionOverlay.dragLineStrokeWidth *= 1.0f + profile.appendageEntry.amplitudeBias * 0.08f;
    scene.actionOverlay.scrollArcStrokeWidth *= 1.0f + profile.overlayEntry.amplitudeBias * 0.06f;
}

} // namespace mousefx::windows
