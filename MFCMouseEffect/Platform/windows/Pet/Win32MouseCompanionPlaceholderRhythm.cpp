#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderRhythm.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

float ClampUnit(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float ClampSigned(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

} // namespace

Win32MouseCompanionPlaceholderRhythm BuildWin32MouseCompanionPlaceholderRhythm(
    const Win32MouseCompanionPlaceholderMotion& motion,
    float facingSign) {
    Win32MouseCompanionPlaceholderRhythm rhythm{};

    const float foreCycle = ClampSigned((motion.frontPawLiftPx - motion.rearPawLiftPx) / 8.0f);
    const float rearCycle = ClampSigned((motion.frontLegLiftPx - motion.rearLegLiftPx) / 7.0f);
    const float earCycle = ClampSigned((motion.frontEarLiftPx - motion.rearEarLiftPx) / 7.5f);
    const float tailCycle = ClampSigned((motion.tailSwingPx + motion.tailLiftPx * 0.3f) / 7.0f);
    const float strideAccent = ClampUnit(
        std::max(std::abs(foreCycle), std::abs(rearCycle)) +
        motion.reactive.dragTension * 0.35f);

    rhythm.foreCycle = foreCycle;
    rhythm.rearCycle = rearCycle;
    rhythm.earCycle = earCycle;
    rhythm.tailCycle = tailCycle;
    rhythm.strideAccent = strideAccent;
    rhythm.silhouetteNudgePx =
        (foreCycle * 1.5f + rearCycle * 1.1f + tailCycle * 0.6f) * facingSign;
    rhythm.earSyncBoost = earCycle * 1.6f + foreCycle * 0.6f;
    rhythm.tailTipLiftPx = std::max(0.0f, tailCycle * 1.8f + strideAccent * 1.2f);
    rhythm.foreBridgeWidth = 1.0f + strideAccent * 0.55f;
    rhythm.rearBridgeWidth = 1.0f + ClampUnit(std::abs(rearCycle)) * 0.45f;
    return rhythm;
}

} // namespace mousefx::windows
