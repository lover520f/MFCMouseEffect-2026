#include "pch.h"

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"

namespace mousefx::macos_click_pulse {
namespace {

ClickEffectProfile BuildComputeProfile(const macos_effect_profile::ClickRenderProfile& profile) {
    ClickEffectProfile out{};
    out.normalSizePx = profile.normalSizePx;
    out.textSizePx = profile.textSizePx;
    out.normalDurationSec = profile.normalDurationSec;
    out.textDurationSec = profile.textDurationSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.left = {profile.leftButton.fillArgb, profile.leftButton.strokeArgb, profile.leftButton.glowArgb};
    out.right = {profile.rightButton.fillArgb, profile.rightButton.strokeArgb, profile.rightButton.glowArgb};
    out.middle = {profile.middleButton.fillArgb, profile.middleButton.strokeArgb, profile.middleButton.glowArgb};
    return out;
}

} // namespace

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
        BuildComputeProfile(profile));
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
