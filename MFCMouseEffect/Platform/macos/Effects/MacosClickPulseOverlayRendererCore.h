#pragma once

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

namespace mousefx::macos_click_pulse {

void ShowClickPulseOverlayOnMain(
    const ClickEffectRenderCommand& command,
    const std::string& themeName);

void ShowClickPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile);

} // namespace mousefx::macos_click_pulse
