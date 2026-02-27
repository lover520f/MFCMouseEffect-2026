#pragma once

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <cstddef>
#include <string>

namespace mousefx::macos_hover_pulse {

void ShowHoverPulseOverlay(const HoverEffectRenderCommand& command, const std::string& themeName);
void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoverRenderProfile& profile);
void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName);
void CloseHoverPulseOverlay();
size_t GetActiveHoverPulseWindowCount();

} // namespace mousefx::macos_hover_pulse
