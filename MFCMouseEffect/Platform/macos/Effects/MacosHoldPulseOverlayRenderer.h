#pragma once

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace mousefx::macos_hold_pulse {

void StartHoldPulseOverlay(const HoldEffectStartCommand& command, const std::string& themeName);
void UpdateHoldPulseOverlay(const HoldEffectUpdateCommand& command, const macos_effect_profile::HoldRenderProfile& profile);
void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoldRenderProfile& profile);
void UpdateHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    uint32_t holdMs,
    const macos_effect_profile::HoldRenderProfile& profile);
void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName);
void UpdateHoldPulseOverlay(const ScreenPoint& overlayPt, uint32_t holdMs);
void StopHoldPulseOverlay();
size_t GetActiveHoldPulseWindowCount();

} // namespace mousefx::macos_hold_pulse
