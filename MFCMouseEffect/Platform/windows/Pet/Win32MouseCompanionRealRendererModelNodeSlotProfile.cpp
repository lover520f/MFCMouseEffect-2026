#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeSlotProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveSlotState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& driveState = runtime.modelAssetNodeDriveProfile.driveState;
    if (driveState == "model_asset_node_drive_bound" && runtime.assets &&
        runtime.assets->modelNodeSlotsReady) {
        return "slot_binding_ready";
    }
    if (driveState == "model_asset_node_drive_pose_ready") {
        return "slot_stub_ready";
    }
    if (driveState == "model_asset_node_drive_ready" ||
        driveState == "model_asset_node_drive_partial") {
        return "slot_scaffold";
    }
    return "preview_only";
}

Win32MouseCompanionRealRendererModelNodeSlotEntry BuildSlotEntry(
    const char* logicalNode,
    const char* slotName,
    float bindWeight,
    bool slotsReady) {
    Win32MouseCompanionRealRendererModelNodeSlotEntry entry{};
    entry.logicalNode = logicalNode ? logicalNode : "";
    entry.slotName = slotName ? slotName : "";
    entry.bindWeight = bindWeight;
    entry.slotReady = slotsReady && bindWeight > 0.0f;
    return entry;
}

uint32_t CountReadySlots(
    const Win32MouseCompanionRealRendererModelNodeSlotProfile& profile) {
    uint32_t count = 0;
    if (profile.bodySlot.slotReady) { ++count; }
    if (profile.headSlot.slotReady) { ++count; }
    if (profile.appendageSlot.slotReady) { ++count; }
    if (profile.overlaySlot.slotReady) { ++count; }
    if (profile.groundingSlot.slotReady) { ++count; }
    return count;
}

std::string BuildSlotSummaryBrief(
    const std::string& state,
    uint32_t slotCount,
    uint32_t readySlotCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        state.empty() ? "preview_only" : state.c_str(),
        slotCount,
        readySlotCount);
    return std::string(buffer);
}

std::string BuildSlotNameBrief(
    const Win32MouseCompanionRealRendererModelNodeSlotProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodySlot.slotName.c_str(),
        profile.headSlot.slotName.c_str(),
        profile.appendageSlot.slotName.c_str(),
        profile.overlaySlot.slotName.c_str(),
        profile.groundingSlot.slotName.c_str());
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelNodeSlotProfile
BuildWin32MouseCompanionRealRendererModelNodeSlotProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererModelNodeSlotProfile profile{};
    profile.slotState = ResolveSlotState(runtime);
    profile.slotCount = 5;

    const bool slotsReady = runtime.assets && runtime.assets->modelNodeSlotsReady;
    const auto& binding = runtime.modelNodeBindingProfile;
    const float driveWeight = runtime.modelAssetNodeDriveProfile.driveWeight;
    profile.bodySlot = BuildSlotEntry(
        "body",
        "body_root",
        binding.bodyEntry.bindWeight * driveWeight,
        slotsReady);
    profile.headSlot = BuildSlotEntry(
        "head",
        "head_anchor",
        binding.headEntry.bindWeight * driveWeight,
        slotsReady);
    profile.appendageSlot = BuildSlotEntry(
        "appendage",
        "appendage_anchor",
        binding.appendageEntry.bindWeight * driveWeight,
        slotsReady);
    profile.overlaySlot = BuildSlotEntry(
        "overlay",
        "overlay_anchor",
        binding.overlayEntry.bindWeight * driveWeight,
        slotsReady);
    profile.groundingSlot = BuildSlotEntry(
        "grounding",
        "grounding_anchor",
        binding.groundingEntry.bindWeight * driveWeight,
        slotsReady);

    profile.readySlotCount = CountReadySlots(profile);
    profile.brief =
        BuildSlotSummaryBrief(profile.slotState, profile.slotCount, profile.readySlotCount);
    profile.slotBrief = BuildSlotNameBrief(profile);
    return profile;
}

} // namespace mousefx::windows
