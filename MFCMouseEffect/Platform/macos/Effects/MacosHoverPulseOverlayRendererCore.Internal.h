#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
struct HoverPulseRenderPlan {
    std::string hoverType{};
    bool tubesMode = false;
    CGFloat size = 0;
    NSRect frame = NSZeroRect;
};

HoverPulseRenderPlan BuildHoverPulseRenderPlan(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const macos_effect_profile::HoverRenderProfile& profile);

void ConfigureHoverRingLayer(
    CAShapeLayer* ring,
    NSView* content,
    const HoverPulseRenderPlan& plan,
    const macos_effect_profile::HoverRenderProfile& profile);

void AddHoverExtraLayersAndAnimations(
    NSView* content,
    const HoverPulseRenderPlan& plan,
    const macos_effect_profile::HoverRenderProfile& profile);
#endif

} // namespace mousefx::macos_hover_pulse
