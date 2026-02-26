#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstddef>
#include <string>

namespace mousefx::macos_hover_pulse {

void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName);
void CloseHoverPulseOverlay();
size_t GetActiveHoverPulseWindowCount();

} // namespace mousefx::macos_hover_pulse
