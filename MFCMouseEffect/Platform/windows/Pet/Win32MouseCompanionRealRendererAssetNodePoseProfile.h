#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile;
struct Win32MouseCompanionRealRendererScene;
struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodePoseEntry final {
    std::string logicalNode;
    std::string assetNodePath;
    float poseX{0.0f};
    float poseY{0.0f};
    float poseScale{1.0f};
    float poseTiltDeg{0.0f};
    float poseWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseProfile final {
    std::string poseState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string valueBrief{
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)"};
};

Win32MouseCompanionRealRendererAssetNodePoseProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene,
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodePoseProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
