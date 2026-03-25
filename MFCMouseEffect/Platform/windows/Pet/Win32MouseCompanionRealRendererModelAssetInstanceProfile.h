#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetResidencyProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetInstanceProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float instanceWeight{0.0f};
    std::string instanceState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string slotBrief{
        "model:stub|action:stub|appearance:stub|pose:stub|adapter:runtime_only"};
    std::string valueBrief{
        "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetInstanceProfile
BuildWin32MouseCompanionRealRendererModelAssetInstanceProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetResidencyProfile& residencyProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetInstanceProfile
BuildWin32MouseCompanionRealRendererModelAssetInstanceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetInstanceProfile(
    const Win32MouseCompanionRealRendererModelAssetInstanceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
