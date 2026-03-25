#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetLoadProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetDecodeProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float decodeWeight{0.0f};
    std::string decodeState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string pipelineBrief{
        "model:stub|action:stub|appearance:stub|transforms:stub|adapter:runtime_only"};
    std::string valueBrief{
        "model:0.00|action:0.00|appearance:0.00|transforms:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetDecodeProfile
BuildWin32MouseCompanionRealRendererModelAssetDecodeProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetLoadProfile& loadProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetDecodeProfile
BuildWin32MouseCompanionRealRendererModelAssetDecodeProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetDecodeProfile(
    const Win32MouseCompanionRealRendererModelAssetDecodeProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
