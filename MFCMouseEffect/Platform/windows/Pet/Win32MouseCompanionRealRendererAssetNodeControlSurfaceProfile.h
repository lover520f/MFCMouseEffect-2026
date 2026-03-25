#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeRigChannelProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry final {
    std::string logicalNode;
    std::string rigChannelName;
    std::string controlSurfaceName;
    float surfaceWeight{0.0f};
    float alphaBias{0.0f};
    float strokeBias{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile final {
    std::string surfaceState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string surfaceBrief{
        "body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& rigChannelProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
