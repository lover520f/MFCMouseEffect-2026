#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetBindingTableProfile;
struct Win32MouseCompanionRealRendererModelAssetNodeLiftProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeBindProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float bindWeight{0.0f};
    std::string bindState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string bindBrief{
        "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeBindProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeLiftProfile& nodeLiftProfile,
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeBindProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeBindProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
