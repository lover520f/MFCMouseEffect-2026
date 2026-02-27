#pragma once

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

namespace mousefx::macos_scroll_pulse {

void CloseAllScrollPulseWindows();
void ShowScrollPulseOverlay(const ScrollEffectRenderCommand& command, const std::string& themeName);
void ShowScrollPulseOverlay(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ScrollRenderProfile& profile);

void ShowScrollPulseOverlay(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName);

} // namespace mousefx::macos_scroll_pulse
