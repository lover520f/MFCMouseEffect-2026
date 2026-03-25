#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeResolveProfile;
struct Win32MouseCompanionRealRendererModelNodeBindingProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeDriveProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float driveWeight{0.0f};
    std::string driveState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string driveBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeDriveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeResolveProfile& nodeResolveProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeDriveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriveProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
