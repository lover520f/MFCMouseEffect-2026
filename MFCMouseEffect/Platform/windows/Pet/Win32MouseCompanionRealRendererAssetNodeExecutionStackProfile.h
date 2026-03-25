#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry final {
    std::string logicalNode;
    std::string busName;
    std::string stackName;
    float stackWeight{0.0f};
    float paintStack{0.0f};
    float compositeStack{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile final {
    std::string stackState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string stackBrief{
        "body:execution.stack.body.shell|head:execution.stack.head.mask|appendage:execution.stack.appendage.trim|overlay:execution.stack.overlay.fx|grounding:execution.stack.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& surfaceCompositionBusProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
