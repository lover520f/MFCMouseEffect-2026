#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"

namespace mousefx::macos_click_pulse {

void ShowClickPulseOverlayOnMain(
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
    const ClickEffectProfile computeProfile = macos_effect_compute_profile::BuildClickProfile(profile);
    const ClickEffectRenderCommand command =
        ComputeClickEffectRenderCommand(overlayPt, button, effectType, computeProfile);
    ShowClickPulseOverlayOnMain(command, themeName);
#endif
}

} // namespace mousefx::macos_click_pulse
