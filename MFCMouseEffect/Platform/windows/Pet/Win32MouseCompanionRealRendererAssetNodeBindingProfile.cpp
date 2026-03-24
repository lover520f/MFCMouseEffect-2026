#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeBindingProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveAssetBindingState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& driverRegistryState =
        runtime.modelAssetNodeDriverRegistryProfile.driverRegistryState;
    if (driverRegistryState == "model_asset_node_driver_registry_bound" && runtime.assets &&
        runtime.assets->assetNodeBindingsReady) {
        return "asset_binding_ready";
    }
    if (driverRegistryState == "model_asset_node_driver_registry_pose_ready") {
        return "asset_binding_stub_ready";
    }
    if (driverRegistryState == "model_asset_node_driver_registry_ready" ||
        driverRegistryState == "model_asset_node_driver_registry_partial") {
        return "asset_binding_scaffold";
    }
    return "preview_only";
}

const char* ResolveAssetNodePath(const std::string& assetNodeName) {
    if (assetNodeName == "asset.body.root") {
        return "/pet/body/root";
    }
    if (assetNodeName == "asset.head.anchor") {
        return "/pet/body/head";
    }
    if (assetNodeName == "asset.appendage.anchor") {
        return "/pet/body/appendage";
    }
    if (assetNodeName == "asset.overlay.anchor") {
        return "/pet/fx/overlay";
    }
    if (assetNodeName == "asset.grounding.anchor") {
        return "/pet/fx/grounding";
    }
    return "/pet/unknown";
}

float ResolveAssetBindingWeight(const std::string& logicalNode, float registryWeight) {
    if (logicalNode == "body") {
        return registryWeight * 0.98f;
    }
    if (logicalNode == "head") {
        return registryWeight * 1.03f;
    }
    if (logicalNode == "appendage") {
        return registryWeight * 1.05f;
    }
    if (logicalNode == "overlay") {
        return registryWeight * 0.97f;
    }
    if (logicalNode == "grounding") {
        return registryWeight * 0.95f;
    }
    return registryWeight;
}

Win32MouseCompanionRealRendererAssetNodeBindingEntry BuildAssetBindingEntry(
    const Win32MouseCompanionRealRendererModelNodeRegistryEntry& registryEntry,
    bool assetBindingsReady) {
    Win32MouseCompanionRealRendererAssetNodeBindingEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.assetNodeName = registryEntry.assetNodeName;
    entry.assetNodePath = ResolveAssetNodePath(registryEntry.assetNodeName);
    entry.bindingWeight =
        ResolveAssetBindingWeight(registryEntry.logicalNode, registryEntry.registryWeight);
    entry.resolved =
        assetBindingsReady && registryEntry.resolved && entry.bindingWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeBindingProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& bindingState,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        bindingState.empty() ? "preview_only" : bindingState.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildPathBrief(
    const Win32MouseCompanionRealRendererAssetNodeBindingProfile& profile) {
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

std::string BuildWeightBrief(
    const Win32MouseCompanionRealRendererAssetNodeBindingProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyEntry.bindingWeight,
        profile.headEntry.bindingWeight,
        profile.appendageEntry.bindingWeight,
        profile.overlayEntry.bindingWeight,
        profile.groundingEntry.bindingWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeBindingProfile
BuildWin32MouseCompanionRealRendererAssetNodeBindingProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeBindingProfile profile{};
    profile.bindingState = ResolveAssetBindingState(runtime);
    profile.entryCount = 5;

    const bool assetBindingsReady = runtime.assets && runtime.assets->assetNodeBindingsReady;
    const auto& registry = runtime.modelNodeRegistryProfile;
    const float executeWeight =
        runtime.modelAssetNodeDriverRegistryProfile.driverRegistryWeight;
    profile.bodyEntry = BuildAssetBindingEntry(registry.bodyEntry, assetBindingsReady);
    profile.bodyEntry.bindingWeight *= executeWeight;
    profile.bodyEntry.resolved = profile.bodyEntry.resolved && profile.bodyEntry.bindingWeight > 0.0f;
    profile.headEntry = BuildAssetBindingEntry(registry.headEntry, assetBindingsReady);
    profile.headEntry.bindingWeight *= executeWeight;
    profile.headEntry.resolved = profile.headEntry.resolved && profile.headEntry.bindingWeight > 0.0f;
    profile.appendageEntry = BuildAssetBindingEntry(registry.appendageEntry, assetBindingsReady);
    profile.appendageEntry.bindingWeight *= executeWeight;
    profile.appendageEntry.resolved = profile.appendageEntry.resolved && profile.appendageEntry.bindingWeight > 0.0f;
    profile.overlayEntry = BuildAssetBindingEntry(registry.overlayEntry, assetBindingsReady);
    profile.overlayEntry.bindingWeight *= executeWeight;
    profile.overlayEntry.resolved = profile.overlayEntry.resolved && profile.overlayEntry.bindingWeight > 0.0f;
    profile.groundingEntry = BuildAssetBindingEntry(registry.groundingEntry, assetBindingsReady);
    profile.groundingEntry.bindingWeight *= executeWeight;
    profile.groundingEntry.resolved = profile.groundingEntry.resolved && profile.groundingEntry.bindingWeight > 0.0f;

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.bindingState, profile.entryCount, profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.weightBrief = BuildWeightBrief(profile);
    return profile;
}

} // namespace mousefx::windows
