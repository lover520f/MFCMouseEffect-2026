#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseChannelProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry final {
    std::string logicalNode;
    std::string channelName;
    std::string constraintName;
    float constraintStrength{0.0f};
    float biasX{0.0f};
    float biasY{0.0f};
    float biasScale{1.0f};
    float biasTiltDeg{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile final {
    std::string constraintState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseConstraintEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string constraintBrief{
        "body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow"};
    std::string valueBrief{
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)"};
};

Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseChannelProfile& channelProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
