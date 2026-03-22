#pragma once

#include <gdiplus.h>

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererMotionProfile final {
    float actionIntensity{0.0f};
    float reactiveIntensity{0.0f};
    float scrollIntensity{0.0f};
    float clickSquash{0.0f};
    float dragLean{0.0f};
    float scrollLean{0.0f};
    float earLift{0.0f};
    float earSwing{0.0f};
    float earSpreadPulse{0.0f};
    float handLift{0.0f};
    float handSwing{0.0f};
    float legStride{0.0f};
    float legLift{0.0f};
    float stateLift{0.0f};
    float headNod{0.0f};
    float bodyForward{0.0f};
    float tailLift{0.0f};
    float tailSwing{0.0f};
    float shadowScale{1.0f};
    float breathLift{0.0f};
    float breathScale{1.0f};
    float idleTailSway{0.0f};
    float idleEarCadence{0.0f};
    float idleHeadSway{0.0f};
    float idleHandFloat{0.0f};
    float blushAlpha{130.0f};
    float eyeOpen{1.0f};
    float pupilFocusX{0.0f};
    float pupilFocusY{0.0f};
    float eyeHighlightAlpha{190.0f};
    float browTilt{0.0f};
    float browLift{0.0f};
    float mouthStartDeg{10.0f};
    float mouthSweepDeg{160.0f};
    float mouthStrokeWidth{1.4f};
    Gdiplus::Color overlayAccentColor{};
};

Win32MouseCompanionRealRendererMotionProfile BuildWin32MouseCompanionRealRendererMotionProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
