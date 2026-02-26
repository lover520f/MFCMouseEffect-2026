#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace mousefx::macos_hold_pulse {

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName);
void UpdateHoldPulseOverlay(const ScreenPoint& overlayPt, uint32_t holdMs);
void StopHoldPulseOverlay();
size_t GetActiveHoldPulseWindowCount();

} // namespace mousefx::macos_hold_pulse
