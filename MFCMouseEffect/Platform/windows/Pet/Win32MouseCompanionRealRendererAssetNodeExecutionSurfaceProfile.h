#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry final {
    std::string logicalNode;
    std::string phaseName;
    std::string surfaceName;
    float surfaceWeight{0.0f};
    float paintDrive{0.0f};
    float compositeDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile final {
    std::string surfaceState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string surfaceBrief{
        "body:execution.surface.body.shell|head:execution.surface.head.mask|appendage:execution.surface.appendage.trim|overlay:execution.surface.overlay.fx|grounding:execution.surface.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& controllerPhaseProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
