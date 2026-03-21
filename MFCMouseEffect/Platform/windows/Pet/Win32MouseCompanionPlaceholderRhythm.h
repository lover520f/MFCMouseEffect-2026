#pragma once

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderRhythm {
    float foreCycle{0.0f};
    float rearCycle{0.0f};
    float earCycle{0.0f};
    float tailCycle{0.0f};
    float silhouetteNudgePx{0.0f};
    float earSyncBoost{0.0f};
    float tailTipLiftPx{0.0f};
    float foreBridgeWidth{0.0f};
    float rearBridgeWidth{0.0f};
    float strideAccent{0.0f};
};

Win32MouseCompanionPlaceholderRhythm BuildWin32MouseCompanionPlaceholderRhythm(
    const Win32MouseCompanionPlaceholderMotion& motion,
    float facingSign);

} // namespace mousefx::windows
