#pragma once

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

namespace mousefx::macos_trail_pulse {

void CloseAllTrailPulseWindows();
void ShowTrailPulseOverlay(const TrailEffectRenderCommand& command, const std::string& themeName);
void ShowTrailPulseOverlay(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::TrailRenderProfile& profile);

void ShowTrailPulseOverlay(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName);

} // namespace mousefx::macos_trail_pulse
