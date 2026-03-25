#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseProfile;

struct Win32MouseCompanionRealRendererAssetNodePoseResolverEntry final {
    std::string logicalNode;
    std::string assetNodePath;
    std::string poseKind;
    float resolvedPoseX{0.0f};
    float resolvedPoseY{0.0f};
    float resolvedPoseScale{1.0f};
    float resolvedPoseTiltDeg{0.0f};
    float resolvedPoseWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseResolverProfile final {
    std::string resolverState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseResolverEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseResolverEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseResolverEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseResolverEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseResolverEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string valueBrief{
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)"};
};

Win32MouseCompanionRealRendererAssetNodePoseResolverProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseResolverProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& poseProfile);

} // namespace mousefx::windows
