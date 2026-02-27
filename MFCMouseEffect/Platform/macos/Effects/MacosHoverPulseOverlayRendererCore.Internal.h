#pragma once

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
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
    HoverEffectRenderCommand command{};
    CGFloat size = 0;
    NSRect frame = NSZeroRect;
    CFTimeInterval breatheDurationSec = 0;
    CFTimeInterval tubesSpinDurationSec = 0;
};

HoverPulseRenderPlan BuildHoverPulseRenderPlan(const HoverEffectRenderCommand& command);

void ConfigureHoverRingLayer(
    CAShapeLayer* ring,
    NSView* content,
    const HoverPulseRenderPlan& plan);

void AddHoverExtraLayersAndAnimations(
    NSView* content,
    const HoverPulseRenderPlan& plan);
#endif

} // namespace mousefx::macos_hover_pulse
