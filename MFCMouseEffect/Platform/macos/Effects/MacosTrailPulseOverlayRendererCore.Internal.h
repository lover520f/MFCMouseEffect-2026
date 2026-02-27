#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
struct TrailPulseRenderPlan {
    std::string trailType{};
    bool tubesMode = false;
    bool particleMode = false;
    bool glowMode = false;
    CGFloat size = 0;
    NSRect frame = NSZeroRect;
    CFTimeInterval durationSec = 0;
    int closeAfterMs = 0;
};

TrailPulseRenderPlan BuildTrailPulseRenderPlan(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const macos_effect_profile::TrailRenderProfile& profile);

void ConfigureTrailCoreLayer(
    CAShapeLayer* core,
    NSView* content,
    const TrailPulseRenderPlan& plan,
    double deltaX,
    double deltaY,
    const macos_effect_profile::TrailRenderProfile& profile);

void AddTrailGlowLayer(
    NSView* content,
    const TrailPulseRenderPlan& plan,
    const macos_effect_profile::TrailRenderProfile& profile);
void StartTrailPulseAnimation(CAShapeLayer* core, const TrailPulseRenderPlan& plan, const macos_effect_profile::TrailRenderProfile& profile);
#endif

} // namespace mousefx::macos_trail_pulse
