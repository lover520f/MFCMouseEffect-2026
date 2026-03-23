#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"

#include "MouseFx/Core/Control/PetVisualAssetCoordinator.h"

namespace mousefx::windows {

Win32MouseCompanionRealRendererAssetResources BuildWin32MouseCompanionRealRendererAssetResources(
    const Win32MouseCompanionRendererInput& input) {
    Win32MouseCompanionRealRendererAssetResources resources{};
    resources.modelPath = input.modelPath;
    resources.modelSourceFormat = ResolvePetVisualModelSourceFormat(input.modelPath);
    resources.actionLibraryPath = input.actionLibraryPath;
    resources.appearanceProfileSkinVariantId = input.appearanceProfile.skinVariantId;
    resources.appearanceAccessoryIds = input.appearanceProfile.enabledAccessoryIds;
    resources.appearanceRequestedPresetId = input.appearanceProfile.requestedPresetId;
    resources.appearanceResolvedPresetId = input.appearanceProfile.resolvedPresetId;
    resources.modelReady = input.modelAssetAvailable && !resources.modelPath.empty();
    resources.modelNodeSlotsReady = resources.modelReady;
    resources.modelNodeRegistryReady = resources.modelNodeSlotsReady;
    resources.assetNodeBindingsReady = resources.modelNodeRegistryReady;
    resources.actionLibraryReady = input.actionLibraryAvailable && !resources.actionLibraryPath.empty();
    resources.appearanceProfileReady = input.appearanceProfile.loaded;
    return resources;
}

} // namespace mousefx::windows
