#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry final {
    std::string logicalNode;
    std::string surfaceName;
    std::string busName;
    float busWeight{0.0f};
    float paintMix{0.0f};
    float compositeMix{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile final {
    std::string busState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string busBrief{
        "body:surface.bus.body.shell|head:surface.bus.head.mask|appendage:surface.bus.appendage.trim|overlay:surface.bus.overlay.fx|grounding:surface.bus.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& executionSurfaceProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
