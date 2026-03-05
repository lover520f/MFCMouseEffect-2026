#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"

#include <cstddef>
#include <string>

namespace mousefx::macos_hold_pulse {

void StartHoldPulseOverlay(const HoldEffectStartCommand& command, const std::string& themeName);
void UpdateHoldPulseOverlay(const HoldEffectUpdateCommand& command);
void StopHoldPulseOverlay();
size_t GetActiveHoldPulseWindowCount();

} // namespace mousefx::macos_hold_pulse
