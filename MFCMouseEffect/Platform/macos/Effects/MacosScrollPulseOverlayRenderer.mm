#include "pch.h"

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

namespace mousefx::macos_scroll_pulse {
namespace {

ScrollEffectProfile BuildComputeProfile(const macos_effect_profile::ScrollRenderProfile& profile) {
    ScrollEffectProfile out{};
    out.verticalSizePx = profile.verticalSizePx;
    out.horizontalSizePx = profile.horizontalSizePx;
    out.baseDurationSec = profile.baseDurationSec;
    out.perStrengthStepSec = profile.perStrengthStepSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.defaultDurationScale = profile.defaultDurationScale;
    out.helixDurationScale = profile.helixDurationScale;
    out.twinkleDurationScale = profile.twinkleDurationScale;
    out.defaultSizeScale = profile.defaultSizeScale;
    out.helixSizeScale = profile.helixSizeScale;
    out.twinkleSizeScale = profile.twinkleSizeScale;
    out.horizontalPositive = {profile.horizontalPositive.fillArgb, profile.horizontalPositive.strokeArgb};
    out.horizontalNegative = {profile.horizontalNegative.fillArgb, profile.horizontalNegative.strokeArgb};
    out.verticalPositive = {profile.verticalPositive.fillArgb, profile.verticalPositive.strokeArgb};
    out.verticalNegative = {profile.verticalNegative.fillArgb, profile.verticalNegative.strokeArgb};
    return out;
}

} // namespace

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
        ComputeScrollEffectRenderCommand(overlayPt, horizontal, delta, effectType, BuildComputeProfile(profile));
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
