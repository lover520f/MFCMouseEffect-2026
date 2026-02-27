#pragma once

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

namespace mousefx::macos_scroll_pulse {

void ShowScrollPulseOverlayOnMain(
    const ScrollEffectRenderCommand& command,
    const std::string& themeName);

void ShowScrollPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ScrollRenderProfile& profile);

} // namespace mousefx::macos_scroll_pulse
