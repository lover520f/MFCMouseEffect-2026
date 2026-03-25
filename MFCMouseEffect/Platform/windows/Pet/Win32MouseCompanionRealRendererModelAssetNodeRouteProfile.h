#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeMountProfile;
struct Win32MouseCompanionRealRendererModelNodeRegistryProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeRouteProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float routeWeight{0.0f};
    std::string routeState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string routeBrief{
        "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeRouteProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeMountProfile& nodeMountProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeRouteProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeRouteProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
