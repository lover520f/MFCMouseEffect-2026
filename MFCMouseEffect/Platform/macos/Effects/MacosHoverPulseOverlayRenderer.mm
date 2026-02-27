#include "pch.h"

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_hover_pulse {
namespace {

HoverEffectProfile BuildComputeProfile(const macos_effect_profile::HoverRenderProfile& profile) {
    HoverEffectProfile out{};
    out.sizePx = profile.sizePx;
    out.breatheDurationSec = profile.breatheDurationSec;
    out.spinDurationSec = profile.spinDurationSec;
    out.baseOpacity = profile.baseOpacity;
    out.glowSizeScale = profile.glowSizeScale;
    out.tubesSizeScale = profile.tubesSizeScale;
    out.glowBreatheScale = profile.glowBreatheScale;
    out.tubesBreatheScale = profile.tubesBreatheScale;
    out.tubesSpinScale = profile.tubesSpinScale;
    out.colors = {profile.colors.glowFillArgb, profile.colors.glowStrokeArgb, profile.colors.tubesStrokeArgb};
    return out;
}

} // namespace

void ShowHoverPulseOverlay(const HoverEffectRenderCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    const HoverEffectRenderCommand commandCopy = command;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowHoverPulseOverlayOnMain(commandCopy, themeCopy);
    });
#endif
}

void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoverRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const HoverEffectRenderCommand command =
        ComputeHoverEffectRenderCommand(overlayPt, effectType, BuildComputeProfile(profile));
    ShowHoverPulseOverlay(command, themeName);
#endif
}

void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName) {
    ShowHoverPulseOverlay(overlayPt, effectType, themeName, macos_effect_profile::DefaultHoverRenderProfile());
}

void CloseHoverPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseHoverPulseOverlayOnMain();
    });
#endif
}

size_t GetActiveHoverPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    __block size_t count = 0;
    macos_overlay_support::RunOnMainThreadSync(^{
      count = GetActiveHoverPulseWindowCountOnMain();
    });
    return count;
#endif
}

} // namespace mousefx::macos_hover_pulse
