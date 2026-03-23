#pragma once

#include <string>

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"

namespace mousefx::windows {

bool IsSupportedWin32MouseCompanionRenderPluginStyleIntent(
    const std::string& value);

bool IsSupportedWin32MouseCompanionRenderPluginSampleTier(
    const std::string& value);

std::string ResolveWin32MouseCompanionRenderPluginDefaultLaneStyleIntent(
    const std::string& defaultLaneCandidate,
    const std::string& declaredStyleIntent,
    Win32MouseCompanionRealRendererAppearanceComboPreset comboPresetOverride);

std::string ResolveWin32MouseCompanionRenderPluginDefaultLaneCandidateTier(
    const std::string& defaultLaneCandidate,
    const std::string& declaredSampleTier);

std::string BuildWin32MouseCompanionRenderPluginContractBrief(
    const std::string& appearanceSemanticsMode,
    const std::string& styleIntent,
    const std::string& sampleTier);

} // namespace mousefx::windows
