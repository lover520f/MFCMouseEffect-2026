#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry final {
    std::string logicalNode;
    std::string phaseName;
    std::string registryName;
    float registryWeight{0.0f};
    float updateBlend{0.0f};
    float settleBlend{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:phase.registry.body.spine|head:phase.registry.head.look|appendage:phase.registry.appendage.reach|overlay:phase.registry.overlay.fx|grounding:phase.registry.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& controllerPhaseProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
