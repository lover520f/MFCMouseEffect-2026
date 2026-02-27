#pragma once

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

namespace mousefx::macos_hover_pulse {

void ShowHoverPulseOverlayOnMain(
    const HoverEffectRenderCommand& command,
    const std::string& themeName);

void ShowHoverPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoverRenderProfile& profile);

void CloseHoverPulseOverlayOnMain();
size_t GetActiveHoverPulseWindowCountOnMain();

} // namespace mousefx::macos_hover_pulse
