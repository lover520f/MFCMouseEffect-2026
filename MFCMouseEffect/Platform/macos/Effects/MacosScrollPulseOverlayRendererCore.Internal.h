#pragma once

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
struct ScrollPulseRenderPlan {
    ScrollEffectRenderCommand command{};
    CGFloat size = 0;
    CGRect bodyRect = CGRectZero;
    NSRect frame = NSZeroRect;
    CFTimeInterval duration = 0;
    int closeAfterMs = 0;
};

ScrollPulseRenderPlan BuildScrollPulseRenderPlan(const ScrollEffectRenderCommand& command);

void AddScrollPulseDecorations(
    NSView* content,
    const ScrollPulseRenderPlan& plan);

void StartScrollPulseAnimation(
    CAShapeLayer* body,
    CAShapeLayer* arrow,
    const ScrollPulseRenderPlan& plan);
#endif

} // namespace mousefx::macos_scroll_pulse
