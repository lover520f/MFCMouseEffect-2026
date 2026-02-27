#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"

namespace mousefx::macos_trail_pulse {
namespace {

TrailEffectProfile BuildComputeProfile(const macos_effect_profile::TrailRenderProfile& profile) {
    TrailEffectProfile out{};
    out.normalSizePx = profile.normalSizePx;
    out.particleSizePx = profile.particleSizePx;
    out.durationSec = profile.durationSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.line = {profile.line.fillArgb, profile.line.strokeArgb};
    out.streamer = {profile.streamer.fillArgb, profile.streamer.strokeArgb};
    out.electric = {profile.electric.fillArgb, profile.electric.strokeArgb};
    out.meteor = {profile.meteor.fillArgb, profile.meteor.strokeArgb};
    out.tubes = {profile.tubes.fillArgb, profile.tubes.strokeArgb};
    out.particle = {profile.particle.fillArgb, profile.particle.strokeArgb};
    out.lineTempo = {profile.lineTempo.durationScale, profile.lineTempo.sizeScale};
    out.streamerTempo = {profile.streamerTempo.durationScale, profile.streamerTempo.sizeScale};
    out.electricTempo = {profile.electricTempo.durationScale, profile.electricTempo.sizeScale};
    out.meteorTempo = {profile.meteorTempo.durationScale, profile.meteorTempo.sizeScale};
    out.tubesTempo = {profile.tubesTempo.durationScale, profile.tubesTempo.sizeScale};
    out.particleTempo = {profile.particleTempo.durationScale, profile.particleTempo.sizeScale};
    return out;
}

} // namespace

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
        ComputeTrailEffectRenderCommand(overlayPt, deltaX, deltaY, effectType, BuildComputeProfile(profile));
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
