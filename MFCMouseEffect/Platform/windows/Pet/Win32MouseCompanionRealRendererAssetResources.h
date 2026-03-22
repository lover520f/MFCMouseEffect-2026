#pragma once

#include <string>
#include <vector>

#include "Platform/windows/Pet/Win32MouseCompanionRendererInput.h"

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources final {
    std::string modelPath;
    std::string modelSourceFormat{"phase1_placeholder"};
    std::string actionLibraryPath;
    std::string appearanceProfileSkinVariantId{"default"};
    std::vector<std::string> appearanceAccessoryIds;
    std::string appearanceRequestedPresetId;
    std::string appearanceResolvedPresetId;
    bool modelReady{false};
    bool actionLibraryReady{false};
    bool appearanceProfileReady{false};
};

Win32MouseCompanionRealRendererAssetResources BuildWin32MouseCompanionRealRendererAssetResources(
    const Win32MouseCompanionRendererInput& input);

} // namespace mousefx::windows
