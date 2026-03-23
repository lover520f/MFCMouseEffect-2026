#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry final {
    std::string logicalNode;
    std::string laneName;
    std::string phaseName;
    float phaseWeight{0.0f};
    float updateDrive{0.0f};
    float settleDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile final {
    std::string phaseState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string phaseBrief{
        "body:controller.phase.body.spine|head:controller.phase.head.look|appendage:controller.phase.appendage.reach|overlay:controller.phase.overlay.fx|grounding:controller.phase.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerPhaseProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& executionLaneProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerPhaseProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
