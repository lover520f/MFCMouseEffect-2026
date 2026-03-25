#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodePoseBusEntry final {
    std::string logicalNode;
    std::string surfaceDriverName;
    std::string poseBusName;
    float busWeight{0.0f};
    float impulseDrive{0.0f};
    float dampingDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseBusProfile final {
    std::string busState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseBusEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseBusEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseBusEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseBusEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseBusEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string busBrief{
        "body:pose.bus.body.spine|head:pose.bus.head.look|appendage:pose.bus.appendage.reach|overlay:pose.bus.overlay.fx|grounding:pose.bus.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodePoseBusProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& surfaceDriverProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
