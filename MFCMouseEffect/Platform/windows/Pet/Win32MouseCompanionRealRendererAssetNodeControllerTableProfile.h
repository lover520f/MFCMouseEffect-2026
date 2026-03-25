#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseBusProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControllerTableEntry final {
    std::string logicalNode;
    std::string poseBusName;
    std::string controllerName;
    float controllerWeight{0.0f};
    float positionDrive{0.0f};
    float emphasisDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControllerTableProfile final {
    std::string tableState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControllerTableEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerTableEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerTableEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerTableEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerTableEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string controllerBrief{
        "body:controller.body.spine|head:controller.head.look|appendage:controller.appendage.reach|overlay:controller.overlay.fx|grounding:controller.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControllerTableProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& poseBusProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
