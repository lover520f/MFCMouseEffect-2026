#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& slotState = runtime.modelNodeSlotProfile.slotState;
    if (slotState == "slot_binding_ready" && runtime.assets &&
        runtime.assets->modelNodeRegistryReady) {
        return "registry_binding_ready";
    }
    if (slotState == "slot_stub_ready") {
        return "registry_stub_ready";
    }
    if (slotState == "slot_scaffold") {
        return "registry_scaffold";
    }
    return "preview_only";
}

float ResolveRegistryWeight(const std::string& logicalNode, float bindWeight) {
    if (logicalNode == "head") {
        return bindWeight * 1.02f;
    }
    if (logicalNode == "appendage") {
        return bindWeight * 1.04f;
    }
    if (logicalNode == "overlay") {
        return bindWeight * 0.96f;
    }
    if (logicalNode == "grounding") {
        return bindWeight * 0.94f;
    }
    return bindWeight;
}

const char* ResolveAssetNodeName(const std::string& slotName) {
    if (slotName == "body_root") {
        return "asset.body.root";
    }
    if (slotName == "head_anchor") {
        return "asset.head.anchor";
    }
    if (slotName == "appendage_anchor") {
        return "asset.appendage.anchor";
    }
    if (slotName == "overlay_anchor") {
        return "asset.overlay.anchor";
    }
    if (slotName == "grounding_anchor") {
        return "asset.grounding.anchor";
    }
    return "asset.unknown";
}

Win32MouseCompanionRealRendererModelNodeRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererModelNodeSlotEntry& slotEntry,
    bool registryReady) {
    Win32MouseCompanionRealRendererModelNodeRegistryEntry entry{};
    entry.logicalNode = slotEntry.logicalNode;
    entry.slotName = slotEntry.slotName;
    entry.assetNodeName = ResolveAssetNodeName(slotEntry.slotName);
    entry.registryWeight = ResolveRegistryWeight(slotEntry.logicalNode, slotEntry.bindWeight);
    entry.resolved = registryReady && slotEntry.slotReady && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildRegistryBrief(
    const std::string& registryState,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        registryState.empty() ? "preview_only" : registryState.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildAssetNodeBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.assetNodeName.c_str(),
        profile.headEntry.assetNodeName.c_str(),
        profile.appendageEntry.assetNodeName.c_str(),
        profile.overlayEntry.assetNodeName.c_str(),
        profile.groundingEntry.assetNodeName.c_str());
    return std::string(buffer);
}

std::string BuildWeightBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyEntry.registryWeight,
        profile.headEntry.registryWeight,
        profile.appendageEntry.registryWeight,
        profile.overlayEntry.registryWeight,
        profile.groundingEntry.registryWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelNodeRegistryProfile
BuildWin32MouseCompanionRealRendererModelNodeRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererModelNodeRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(runtime);
    profile.entryCount = 5;

    const bool registryReady = runtime.assets && runtime.assets->modelNodeRegistryReady;
    const auto& slots = runtime.modelNodeSlotProfile;
    profile.bodyEntry = BuildRegistryEntry(slots.bodySlot, registryReady);
    profile.headEntry = BuildRegistryEntry(slots.headSlot, registryReady);
    profile.appendageEntry = BuildRegistryEntry(slots.appendageSlot, registryReady);
    profile.overlayEntry = BuildRegistryEntry(slots.overlaySlot, registryReady);
    profile.groundingEntry = BuildRegistryEntry(slots.groundingSlot, registryReady);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief =
        BuildRegistryBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.assetNodeBrief = BuildAssetNodeBrief(profile);
    profile.weightBrief = BuildWeightBrief(profile);
    return profile;
}

} // namespace mousefx::windows
