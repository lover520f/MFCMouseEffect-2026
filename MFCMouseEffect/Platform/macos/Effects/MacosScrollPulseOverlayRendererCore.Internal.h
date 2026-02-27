#pragma once

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
    std::string normalizedType{};
    bool helixMode = false;
    bool twinkleMode = false;
    int strengthLevel = 0;
    CGFloat size = 0;
    CGRect bodyRect = CGRectZero;
    NSRect frame = NSZeroRect;
    CFTimeInterval duration = 0;
    int closeAfterMs = 0;
};

ScrollPulseRenderPlan BuildScrollPulseRenderPlan(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const macos_effect_profile::ScrollRenderProfile& profile);

void AddScrollPulseDecorations(
    NSView* content,
    const ScrollPulseRenderPlan& plan,
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile);

void StartScrollPulseAnimation(
    CAShapeLayer* body,
    CAShapeLayer* arrow,
    const ScrollPulseRenderPlan& plan,
    const macos_effect_profile::ScrollRenderProfile& profile);
#endif

} // namespace mousefx::macos_scroll_pulse
