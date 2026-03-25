#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry final {
    std::string logicalNode;
    std::string busName;
    std::string registryName;
    float registryWeight{0.0f};
    float paintRegistry{0.0f};
    float compositeRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:composition.registry.body.shell|head:composition.registry.head.mask|appendage:composition.registry.appendage.trim|overlay:composition.registry.overlay.fx|grounding:composition.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& surfaceCompositionBusProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
