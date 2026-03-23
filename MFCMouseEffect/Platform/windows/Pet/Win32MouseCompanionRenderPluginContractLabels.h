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

} // namespace mousefx::windows
