#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.h"

namespace mousefx::macos_scroll_pulse {

void ShowScrollPulseOverlayOnMain(
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
    const ScrollEffectProfile computeProfile{
        profile.verticalSizePx,
        profile.horizontalSizePx,
        profile.baseDurationSec,
        profile.perStrengthStepSec,
        profile.closePaddingMs,
        profile.baseOpacity,
        profile.defaultDurationScale,
        profile.helixDurationScale,
        profile.twinkleDurationScale,
        profile.defaultSizeScale,
        profile.helixSizeScale,
        profile.twinkleSizeScale,
        {profile.horizontalPositive.fillArgb, profile.horizontalPositive.strokeArgb},
        {profile.horizontalNegative.fillArgb, profile.horizontalNegative.strokeArgb},
        {profile.verticalPositive.fillArgb, profile.verticalPositive.strokeArgb},
        {profile.verticalNegative.fillArgb, profile.verticalNegative.strokeArgb},
    };
    const ScrollEffectRenderCommand command =
        ComputeScrollEffectRenderCommand(overlayPt, horizontal, delta, effectType, computeProfile);
    ShowScrollPulseOverlayOnMain(command, themeName);
#endif
}

} // namespace mousefx::macos_scroll_pulse
