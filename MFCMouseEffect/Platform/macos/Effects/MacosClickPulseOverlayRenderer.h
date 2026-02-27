#pragma once

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

namespace mousefx::macos_click_pulse {

void CloseAllClickPulseWindows();
void ShowClickPulseOverlay(const ClickEffectRenderCommand& command, const std::string& themeName);
void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile);

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName);

} // namespace mousefx::macos_click_pulse
