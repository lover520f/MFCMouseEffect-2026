#pragma once

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
struct ClickPulseRenderPlan {
    ClickEffectRenderCommand command{};
    CGFloat size = 0;
    CGFloat inset = 0;
    NSRect frame = NSZeroRect;
    CFTimeInterval animationDuration = 0;
};

ClickPulseRenderPlan BuildClickPulseRenderPlan(
    const ClickEffectRenderCommand& command);

void ConfigureClickPulseBaseLayer(
    CAShapeLayer* base,
    NSView* content,
    const ClickPulseRenderPlan& plan);

void AddClickPulseExtraLayers(
    NSView* content,
    const ClickPulseRenderPlan& plan);

void StartClickPulseAnimation(
    CAShapeLayer* base,
    const ClickPulseRenderPlan& plan);

int64_t ComputeClickPulseCloseDelayNs(const ClickPulseRenderPlan& plan);
#endif

} // namespace mousefx::macos_click_pulse
