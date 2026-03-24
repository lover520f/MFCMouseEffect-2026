#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"

#include "MouseFx/Core/Control/PetVisualAssetCoordinator.h"

namespace mousefx::windows {
namespace {

std::string BasenameOfPath(const std::string& path) {
    if (path.empty()) {
        return {};
    }
    const size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string StemOfFilename(const std::string& filename) {
    if (filename.empty()) {
        return {};
    }
    const size_t pos = filename.find_last_of('.');
    return pos == std::string::npos ? filename : filename.substr(0, pos);
}

std::string ResolveModelRootNodeKey(
    const std::string& modelFileName,
    const std::string& sourceFormat) {
    const std::string stem = StemOfFilename(modelFileName);
    if (stem.empty()) {
        return "preview_root";
    }
    if (sourceFormat == "vrm") {
        return "vrm_root:" + stem;
    }
    if (sourceFormat == "fbx") {
        return "fbx_root:" + stem;
    }
    if (sourceFormat == "glb" || sourceFormat == "gltf") {
        return "scene_root:" + stem;
    }
    return "preview_root:" + stem;
}

std::string ResolveModelNodeSelectorPrefix(
    const std::string& modelRootNodeKey,
    const std::string& sourceFormat) {
    if (sourceFormat == "vrm") {
        return "/asset/vrm/" + modelRootNodeKey;
    }
    if (sourceFormat == "fbx") {
        return "/asset/fbx/" + modelRootNodeKey;
    }
    if (sourceFormat == "glb" || sourceFormat == "gltf") {
        return "/asset/scene/" + modelRootNodeKey;
    }
    return "/preview/model/" + modelRootNodeKey;
}

} // namespace

Win32MouseCompanionRealRendererAssetResources BuildWin32MouseCompanionRealRendererAssetResources(
    const Win32MouseCompanionRendererInput& input) {
    Win32MouseCompanionRealRendererAssetResources resources{};
    resources.modelPath = input.modelPath;
    resources.modelSourceFormat = ResolvePetVisualModelSourceFormat(input.modelPath);
    resources.modelFileName = BasenameOfPath(resources.modelPath);
    resources.modelRootNodeKey =
        ResolveModelRootNodeKey(resources.modelFileName, resources.modelSourceFormat);
    resources.modelNodeSelectorPrefix =
        ResolveModelNodeSelectorPrefix(resources.modelRootNodeKey, resources.modelSourceFormat);
    resources.actionLibraryPath = input.actionLibraryPath;
    resources.appearanceProfileSkinVariantId = input.appearanceProfile.skinVariantId;
    resources.appearanceAccessoryIds = input.appearanceProfile.enabledAccessoryIds;
    resources.appearanceRequestedPresetId = input.appearanceProfile.requestedPresetId;
    resources.appearanceResolvedPresetId = input.appearanceProfile.resolvedPresetId;
    resources.modelReady = input.modelAssetAvailable && !resources.modelPath.empty();
    resources.modelNodeSlotsReady = resources.modelReady;
    resources.modelNodeRegistryReady = resources.modelNodeSlotsReady;
    resources.assetNodeBindingsReady = resources.modelNodeRegistryReady;
    resources.assetNodeTransformsReady = resources.assetNodeBindingsReady;
    resources.actionLibraryReady = input.actionLibraryAvailable && !resources.actionLibraryPath.empty();
    resources.appearanceProfileReady = input.appearanceProfile.loaded;
    return resources;
}

} // namespace mousefx::windows
