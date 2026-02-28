#include "pch.h"

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"

namespace mousefx::macos_click_pulse {

void CloseAllClickPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseAllClickPulseWindowsNow();
    });
#endif
}

void ShowClickPulseOverlay(const ClickEffectRenderCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    const ClickEffectRenderCommand commandCopy = command;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowClickPulseOverlayOnMain(commandCopy, themeCopy);
    });
#endif
}

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
        overlayPt,
        button,
        effectType,
        macos_effect_compute_profile::BuildClickProfile(profile));
    ShowClickPulseOverlay(command, themeName);
#endif
}

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    ShowClickPulseOverlay(overlayPt, button, effectType, themeName, macos_effect_profile::DefaultClickRenderProfile());
}

} // namespace mousefx::macos_click_pulse
