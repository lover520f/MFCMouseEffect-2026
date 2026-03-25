#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseResolverProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry final {
    std::string logicalNode;
    std::string poseNodeName;
    std::string assetNodePath;
    float registryWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string poseNodeBrief{
        "body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor"};
    std::string weightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
};

Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseResolverProfile& resolverProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
