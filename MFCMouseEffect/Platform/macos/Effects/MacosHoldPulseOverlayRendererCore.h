#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

namespace mousefx::macos_hold_pulse {

void StartHoldPulseOverlayOnMain(
    const HoldEffectStartCommand& command,
    const std::string& themeName);

void StartHoldPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoldRenderProfile& profile);

void UpdateHoldPulseOverlayOnMain(const HoldEffectUpdateCommand& command);
void UpdateHoldPulseOverlayOnMain(const ScreenPoint& overlayPt, uint32_t holdMs);
void CloseHoldPulseOverlayOnMain();
size_t GetActiveHoldPulseWindowCountOnMain();

} // namespace mousefx::macos_hold_pulse
