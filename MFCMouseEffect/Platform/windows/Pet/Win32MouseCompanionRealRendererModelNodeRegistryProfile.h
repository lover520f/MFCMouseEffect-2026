#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererModelNodeRegistryEntry final {
    std::string logicalNode;
    std::string slotName;
    std::string assetNodeName;
    float registryWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererModelNodeRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererModelNodeRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererModelNodeRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererModelNodeRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererModelNodeRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererModelNodeRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string assetNodeBrief{
        "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor"};
    std::string weightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
};

Win32MouseCompanionRealRendererModelNodeRegistryProfile
BuildWin32MouseCompanionRealRendererModelNodeRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
