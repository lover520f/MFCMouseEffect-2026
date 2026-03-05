#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace mousefx::macos_hold_pulse {

void StartHoldPulseOverlayOnMain(
    const HoldEffectStartCommand& command,
    const std::string& themeName);

void UpdateHoldPulseOverlayOnMain(const HoldEffectUpdateCommand& command);
void CloseHoldPulseOverlayOnMain();
size_t GetActiveHoldPulseWindowCountOnMain();

} // namespace mousefx::macos_hold_pulse
