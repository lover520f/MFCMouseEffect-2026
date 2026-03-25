#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeBindProfile;
struct Win32MouseCompanionRealRendererModelNodeAdapterProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeResolveProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float resolveWeight{0.0f};
    std::string resolveState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string resolveBrief{
        "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeResolveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeBindProfile& nodeBindProfile,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeResolveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeResolveProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
