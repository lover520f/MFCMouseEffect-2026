#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"

namespace mousefx::macos_trail_pulse {

void ShowTrailPulseOverlayOnMain(
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
    TrailEffectProfile computeProfile{};
    computeProfile.normalSizePx = profile.normalSizePx;
    computeProfile.particleSizePx = profile.particleSizePx;
    computeProfile.lineWidthPx = profile.lineWidthPx;
    computeProfile.durationSec = profile.durationSec;
    computeProfile.closePaddingMs = profile.closePaddingMs;
    computeProfile.baseOpacity = profile.baseOpacity;
    computeProfile.line = {profile.line.fillArgb, profile.line.strokeArgb};
    computeProfile.streamer = {profile.streamer.fillArgb, profile.streamer.strokeArgb};
    computeProfile.electric = {profile.electric.fillArgb, profile.electric.strokeArgb};
    computeProfile.meteor = {profile.meteor.fillArgb, profile.meteor.strokeArgb};
    computeProfile.tubes = {profile.tubes.fillArgb, profile.tubes.strokeArgb};
    computeProfile.particle = {profile.particle.fillArgb, profile.particle.strokeArgb};
    computeProfile.lineTempo = {profile.lineTempo.durationScale, profile.lineTempo.sizeScale};
    computeProfile.streamerTempo = {profile.streamerTempo.durationScale, profile.streamerTempo.sizeScale};
    computeProfile.electricTempo = {profile.electricTempo.durationScale, profile.electricTempo.sizeScale};
    computeProfile.meteorTempo = {profile.meteorTempo.durationScale, profile.meteorTempo.sizeScale};
    computeProfile.tubesTempo = {profile.tubesTempo.durationScale, profile.tubesTempo.sizeScale};
    computeProfile.particleTempo = {profile.particleTempo.durationScale, profile.particleTempo.sizeScale};
    const TrailEffectRenderCommand command =
        ComputeTrailEffectRenderCommand(overlayPt, deltaX, deltaY, effectType, computeProfile);
    ShowTrailPulseOverlayOnMain(command, themeName);
#endif
}

} // namespace mousefx::macos_trail_pulse
