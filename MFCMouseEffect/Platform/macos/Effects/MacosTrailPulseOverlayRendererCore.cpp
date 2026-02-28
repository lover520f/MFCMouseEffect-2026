#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_trail_pulse {

void ShowTrailPulseOverlayOnMain(
    const TrailEffectRenderCommand& command,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    (void)themeName;
    if (!command.emit) {
        return;
    }
    const TrailPulseRenderPlan plan = BuildTrailPulseRenderPlan(command);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, command.overlayPoint);

    CAShapeLayer* core = [CAShapeLayer layer];
    ConfigureTrailCoreLayer(core, content, plan, command.deltaX, command.deltaY);
    [content.layer addSublayer:core];
    AddTrailGlowLayer(content, plan);
    StartTrailPulseAnimation(core, plan);

    RegisterTrailPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(
            DISPATCH_TIME_NOW,
            static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeTrailPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(window));
        });
#endif
}

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
    const TrailEffectProfile computeProfile{
        profile.normalSizePx,
        profile.particleSizePx,
        profile.durationSec,
        profile.closePaddingMs,
        profile.baseOpacity,
        {profile.line.fillArgb, profile.line.strokeArgb},
        {profile.streamer.fillArgb, profile.streamer.strokeArgb},
        {profile.electric.fillArgb, profile.electric.strokeArgb},
        {profile.meteor.fillArgb, profile.meteor.strokeArgb},
        {profile.tubes.fillArgb, profile.tubes.strokeArgb},
        {profile.particle.fillArgb, profile.particle.strokeArgb},
        {profile.lineTempo.durationScale, profile.lineTempo.sizeScale},
        {profile.streamerTempo.durationScale, profile.streamerTempo.sizeScale},
        {profile.electricTempo.durationScale, profile.electricTempo.sizeScale},
        {profile.meteorTempo.durationScale, profile.meteorTempo.sizeScale},
        {profile.tubesTempo.durationScale, profile.tubesTempo.sizeScale},
        {profile.particleTempo.durationScale, profile.particleTempo.sizeScale},
    };
    const TrailEffectRenderCommand command =
        ComputeTrailEffectRenderCommand(overlayPt, deltaX, deltaY, effectType, computeProfile);
    ShowTrailPulseOverlayOnMain(command, themeName);
#endif
}

} // namespace mousefx::macos_trail_pulse
