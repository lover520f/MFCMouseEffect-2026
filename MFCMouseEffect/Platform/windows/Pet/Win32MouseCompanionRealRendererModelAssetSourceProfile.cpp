#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSourceProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <array>
#include <cstdio>

namespace mousefx::windows {
namespace {

bool IsSupportedModelAssetSourceFormat(const std::string& format) {
    static constexpr std::array<const char*, 4> kSupportedFormats = {
        "glb",
        "gltf",
        "vrm",
        "fbx",
    };
    for (const char* candidate : kSupportedFormats) {
        if (format == candidate) {
            return true;
        }
    }
    return false;
}

std::string BasenameOfPath(const std::string& path) {
    if (path.empty()) {
        return "-";
    }
    const size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

float ResolveSourceReadiness(
    bool modelSourceReady,
    bool sourceFormatSupported,
    bool actionLibraryReady,
    bool appearanceProfileReady,
    const std::string& adapterMode) {
    if (!modelSourceReady || !sourceFormatSupported) {
        return modelSourceReady ? 0.24f : 0.0f;
    }
    float readiness = 0.42f;
    if (actionLibraryReady) {
        readiness += 0.20f;
    }
    if (appearanceProfileReady) {
        readiness += 0.18f;
    }
    if (adapterMode == "pose_unbound") {
        readiness += 0.08f;
    } else if (adapterMode == "pose_bound") {
        readiness += 0.16f;
    }
    return std::clamp(readiness, 0.0f, 1.0f);
}

std::string ResolveSourceState(
    bool modelSourceReady,
    bool sourceFormatSupported,
    bool actionLibraryReady,
    bool appearanceProfileReady,
    const std::string& adapterMode) {
    if (!modelSourceReady) {
        return "preview_only";
    }
    if (!sourceFormatSupported) {
        return "model_asset_stub_ready";
    }
    if (actionLibraryReady && appearanceProfileReady) {
        if (adapterMode == "pose_bound") {
            return "model_asset_bound_ready";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_pose_ready";
        }
        return "model_asset_manifest_ready";
    }
    if (actionLibraryReady || appearanceProfileReady) {
        return "model_asset_source_ready";
    }
    return "model_asset_stub_ready";
}

std::string BuildSourceBrief(
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& profile,
    const std::string& modelSourceFormat) {
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%s/model:%u/action:%u/appearance:%u",
        profile.sourceState.empty() ? "preview_only" : profile.sourceState.c_str(),
        modelSourceFormat.empty() ? "unknown" : modelSourceFormat.c_str(),
        profile.modelSourceReady ? 1u : 0u,
        profile.actionLibraryReady ? 1u : 0u,
        profile.appearanceProfileReady ? 1u : 0u);
    return std::string(buffer);
}

std::string BuildPathBrief(const Win32MouseCompanionRealRendererAssetResources& assets) {
    return "model:" + BasenameOfPath(assets.modelPath) +
           "|action:" + BasenameOfPath(assets.actionLibraryPath) +
           "|appearance:" +
           (assets.appearanceResolvedPresetId.empty()
                ? assets.appearanceProfileSkinVariantId
                : assets.appearanceResolvedPresetId);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& profile,
    const std::string& modelSourceFormat) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "format:%s|readiness:%.2f",
        modelSourceFormat.empty() ? "unknown" : modelSourceFormat.c_str(),
        profile.sourceReadiness);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetSourceProfile
BuildWin32MouseCompanionRealRendererModelAssetSourceProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetSourceProfile profile{};
    profile.modelSourceReady = assets.modelReady && !assets.modelPath.empty();
    profile.actionLibraryReady = assets.actionLibraryReady && !assets.actionLibraryPath.empty();
    profile.appearanceProfileReady = assets.appearanceProfileReady;
    profile.sourceFormatSupported =
        profile.modelSourceReady &&
        IsSupportedModelAssetSourceFormat(assets.modelSourceFormat);
    profile.sourceReadiness = ResolveSourceReadiness(
        profile.modelSourceReady,
        profile.sourceFormatSupported,
        profile.actionLibraryReady,
        profile.appearanceProfileReady,
        adapterMode);
    profile.sourceState = ResolveSourceState(
        profile.modelSourceReady,
        profile.sourceFormatSupported,
        profile.actionLibraryReady,
        profile.appearanceProfileReady,
        adapterMode);
    profile.brief = BuildSourceBrief(profile, assets.modelSourceFormat);
    profile.pathBrief = BuildPathBrief(assets);
    profile.valueBrief = BuildValueBrief(profile, assets.modelSourceFormat);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetSourceProfile
BuildWin32MouseCompanionRealRendererModelAssetSourceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetSourceProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetSourceProfile(
        *runtime.assets,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetSourceProfile(
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.sourceReadiness * 16.0f,
        0.0f,
        255.0f);
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.sourceReadiness * 10.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.sourceReadiness * 0.030f;
    scene.pedestalAlphaScale *= 1.0f + profile.sourceReadiness * 0.024f;
}

} // namespace mousefx::windows
