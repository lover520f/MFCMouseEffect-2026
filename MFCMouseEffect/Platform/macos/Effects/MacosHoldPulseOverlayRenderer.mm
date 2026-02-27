#include "pch.h"

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_hold_pulse {
namespace {

HoldEffectProfile BuildComputeProfile(const macos_effect_profile::HoldRenderProfile& profile) {
    HoldEffectProfile out{};
    out.sizePx = profile.sizePx;
    out.progressFullMs = profile.progressFullMs;
    out.breatheDurationSec = profile.breatheDurationSec;
    out.rotateDurationSec = profile.rotateDurationSec;
    out.rotateDurationFastSec = profile.rotateDurationFastSec;
    out.baseOpacity = profile.baseOpacity;
    out.colors.leftBaseStrokeArgb = profile.colors.leftBaseStrokeArgb;
    out.colors.rightBaseStrokeArgb = profile.colors.rightBaseStrokeArgb;
    out.colors.middleBaseStrokeArgb = profile.colors.middleBaseStrokeArgb;
    out.colors.lightningStrokeArgb = profile.colors.lightningStrokeArgb;
    out.colors.hexStrokeArgb = profile.colors.hexStrokeArgb;
    out.colors.hologramStrokeArgb = profile.colors.hologramStrokeArgb;
    out.colors.quantumHaloStrokeArgb = profile.colors.quantumHaloStrokeArgb;
    out.colors.fluxFieldStrokeArgb = profile.colors.fluxFieldStrokeArgb;
    out.colors.techNeonStrokeArgb = profile.colors.techNeonStrokeArgb;
    return out;
}

} // namespace

void StartHoldPulseOverlay(const HoldEffectStartCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    const HoldEffectStartCommand commandCopy = command;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      StartHoldPulseOverlayOnMain(commandCopy, themeCopy);
    });
#endif
}

void UpdateHoldPulseOverlay(const HoldEffectUpdateCommand& command, const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)command;
    (void)profile;
    return;
#else
    const HoldEffectUpdateCommand commandCopy = command;
    macos_overlay_support::RunOnMainThreadAsync(^{
      (void)profile;
      UpdateHoldPulseOverlayOnMain(commandCopy);
    });
#endif
}

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const HoldEffectStartCommand command =
        ComputeHoldEffectStartCommand(overlayPt, button, effectType, BuildComputeProfile(profile));
    StartHoldPulseOverlay(command, themeName);
#endif
}

void UpdateHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    uint32_t holdMs,
    const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)holdMs;
    (void)profile;
    return;
#else
    HoldEffectUpdateCommand command{};
    command.emit = true;
    command.overlayPoint = overlayPt;
    command.holdMs = holdMs;
    UpdateHoldPulseOverlay(command, profile);
#endif
}

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    StartHoldPulseOverlay(overlayPt, button, effectType, themeName, macos_effect_profile::DefaultHoldRenderProfile());
}

void UpdateHoldPulseOverlay(const ScreenPoint& overlayPt, uint32_t holdMs) {
    UpdateHoldPulseOverlay(overlayPt, holdMs, macos_effect_profile::DefaultHoldRenderProfile());
}

void StopHoldPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseHoldPulseOverlayOnMain();
    });
#endif
}

size_t GetActiveHoldPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    __block size_t count = 0;
    macos_overlay_support::RunOnMainThreadSync(^{
      count = GetActiveHoldPulseWindowCountOnMain();
    });
    return count;
#endif
}

} // namespace mousefx::macos_hold_pulse
