#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodePoseSolveEntry final {
    std::string logicalNode;
    std::string constraintName;
    std::string assetNodePath;
    float solvedPoseX{0.0f};
    float solvedPoseY{0.0f};
    float solvedPoseScale{1.0f};
    float solvedPoseTiltDeg{0.0f};
    float solvedPoseWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseSolveProfile final {
    std::string solveState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseSolveEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseSolveEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseSolveEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseSolveEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseSolveEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string valueBrief{
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)"};
};

Win32MouseCompanionRealRendererAssetNodePoseSolveProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile& constraintProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
