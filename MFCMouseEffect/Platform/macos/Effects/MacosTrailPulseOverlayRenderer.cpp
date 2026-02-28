#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"

namespace mousefx::macos_trail_pulse {

void CloseAllTrailPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseAllTrailPulseWindowsNow();
    });
#endif
}

void ShowTrailPulseOverlay(const TrailEffectRenderCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    const TrailEffectRenderCommand commandCopy = command;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowTrailPulseOverlayOnMain(commandCopy, themeCopy);
    });
#endif
}

void ShowTrailPulseOverlay(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::TrailRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)deltaX;
    (void)deltaY;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const TrailEffectRenderCommand command =
        ComputeTrailEffectRenderCommand(
            overlayPt,
            deltaX,
            deltaY,
            effectType,
            macos_effect_compute_profile::BuildTrailProfile(profile));
    ShowTrailPulseOverlay(command, themeName);
#endif
}

void ShowTrailPulseOverlay(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName) {
    ShowTrailPulseOverlay(overlayPt, deltaX, deltaY, effectType, themeName, macos_effect_profile::DefaultTrailRenderProfile(effectType));
}

} // namespace mousefx::macos_trail_pulse
