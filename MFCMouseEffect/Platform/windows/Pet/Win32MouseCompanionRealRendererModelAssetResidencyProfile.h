#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetDecodeProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetResidencyProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float residencyWeight{0.0f};
    std::string residencyState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string cacheBrief{"model:cold|action:cold|appearance:cold|pose:cold|adapter:runtime_only"};
    std::string valueBrief{"model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetResidencyProfile
BuildWin32MouseCompanionRealRendererModelAssetResidencyProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetDecodeProfile& decodeProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetResidencyProfile
BuildWin32MouseCompanionRealRendererModelAssetResidencyProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetResidencyProfile(
    const Win32MouseCompanionRealRendererModelAssetResidencyProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
