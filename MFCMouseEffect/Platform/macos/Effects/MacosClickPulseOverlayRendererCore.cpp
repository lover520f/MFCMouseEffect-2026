#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_click_pulse {

void ShowClickPulseOverlayOnMain(
    const ClickEffectRenderCommand& command,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    (void)themeName;

    const ClickPulseRenderPlan plan = BuildClickPulseRenderPlan(command);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, command.overlayPoint);

    CAShapeLayer* base = [CAShapeLayer layer];
    base.frame = content.bounds;
    ConfigureClickPulseBaseLayer(base, content, plan);
    [content.layer addSublayer:base];

    AddClickPulseExtraLayers(content, plan);
    StartClickPulseAnimation(base, plan);

    RegisterClickPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(
            DISPATCH_TIME_NOW,
            ComputeClickPulseCloseDelayNs(plan)),
        dispatch_get_main_queue(),
        ^{
          if (!TakeClickPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(window));
        });
#endif
}

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
    const ClickEffectProfile computeProfile{
        profile.normalSizePx,
        profile.textSizePx,
        profile.normalDurationSec,
        profile.textDurationSec,
        profile.closePaddingMs,
        profile.baseOpacity,
        {profile.leftButton.fillArgb, profile.leftButton.strokeArgb, profile.leftButton.glowArgb},
        {profile.rightButton.fillArgb, profile.rightButton.strokeArgb, profile.rightButton.glowArgb},
        {profile.middleButton.fillArgb, profile.middleButton.strokeArgb, profile.middleButton.glowArgb},
    };
    const ClickEffectRenderCommand command =
        ComputeClickEffectRenderCommand(overlayPt, button, effectType, computeProfile);
    ShowClickPulseOverlayOnMain(command, themeName);
#endif
}

} // namespace mousefx::macos_click_pulse
