#include "pch.h"

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

namespace mousefx::macos_scroll_pulse {

void CloseAllScrollPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseAllScrollPulseWindowsNow();
    });
#endif
}

void ShowScrollPulseOverlay(const ScrollEffectRenderCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    const ScrollEffectRenderCommand commandCopy = command;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowScrollPulseOverlayOnMain(commandCopy, themeCopy);
    });
#endif
}

void ShowScrollPulseOverlay(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ScrollRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)horizontal;
    (void)delta;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const ScrollEffectRenderCommand command =
        ComputeScrollEffectRenderCommand(
            overlayPt,
            horizontal,
            delta,
            effectType,
            macos_effect_compute_profile::BuildScrollProfile(profile));
    ShowScrollPulseOverlay(command, themeName);
#endif
}

void ShowScrollPulseOverlay(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName) {
    ShowScrollPulseOverlay(overlayPt, horizontal, delta, effectType, themeName, macos_effect_profile::DefaultScrollRenderProfile());
}

} // namespace mousefx::macos_scroll_pulse
