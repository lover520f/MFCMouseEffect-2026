#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeAttachProfile;
struct Win32MouseCompanionRealRendererPoseAdapterProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeLiftProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float liftWeight{0.0f};
    std::string liftState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string liftBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeLiftProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& nodeAttachProfile,
    const Win32MouseCompanionRealRendererPoseAdapterProfile& poseAdapterProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeLiftProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeLiftProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
