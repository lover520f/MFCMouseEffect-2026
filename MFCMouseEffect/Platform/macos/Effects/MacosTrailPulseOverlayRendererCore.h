#pragma once

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

namespace mousefx::macos_trail_pulse {

void ShowTrailPulseOverlayOnMain(
    const TrailEffectRenderCommand& command,
    const std::string& themeName);

void ShowTrailPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::TrailRenderProfile& profile);

} // namespace mousefx::macos_trail_pulse
