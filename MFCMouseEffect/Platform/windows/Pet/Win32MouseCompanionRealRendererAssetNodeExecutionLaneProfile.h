#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string laneName;
    float laneWeight{0.0f};
    float executeDrive{0.0f};
    float renderDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile final {
    std::string laneState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string laneBrief{
        "body:execution.lane.body.spine|head:execution.lane.head.look|appendage:execution.lane.appendage.reach|overlay:execution.lane.overlay.fx|grounding:execution.lane.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionLaneProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& controllerDriverRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionLaneProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
