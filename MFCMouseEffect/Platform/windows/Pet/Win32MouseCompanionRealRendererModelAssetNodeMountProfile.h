#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeDriveProfile;
struct Win32MouseCompanionRealRendererModelNodeRegistryProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeMountProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float mountWeight{0.0f};
    std::string mountState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string mountBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeMountProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriveProfile& nodeDriveProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeMountProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeMountProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
