#pragma once

#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetSourceProfile final {
    bool modelSourceReady{false};
    bool actionLibraryReady{false};
    bool appearanceProfileReady{false};
    bool sourceFormatSupported{false};
    float sourceReadiness{0.0f};
    std::string sourceState{"preview_only"};
    std::string brief{"preview_only/unknown/model:0/action:0/appearance:0"};
    std::string pathBrief{"model:-|action:-|appearance:default"};
    std::string valueBrief{"format:unknown|readiness:0.00"};
};

Win32MouseCompanionRealRendererModelAssetSourceProfile
BuildWin32MouseCompanionRealRendererModelAssetSourceProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetSourceProfile
BuildWin32MouseCompanionRealRendererModelAssetSourceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetSourceProfile(
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
