#pragma once

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
    std::string normalizedType{};
    bool textMode = false;
    bool starMode = false;
    CGFloat size = 0;
    CGFloat inset = 0;
    NSRect frame = NSZeroRect;
    CFTimeInterval animationDuration = 0;
};

ClickPulseRenderPlan BuildClickPulseRenderPlan(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const macos_effect_profile::ClickRenderProfile& profile);

void ConfigureClickPulseBaseLayer(
    CAShapeLayer* base,
    NSView* content,
    MouseButton button,
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile);

void AddClickPulseExtraLayers(
    NSView* content,
    MouseButton button,
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile);

void StartClickPulseAnimation(
    CAShapeLayer* base,
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile);

int64_t ComputeClickPulseCloseDelayNs(
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile);
#endif

} // namespace mousefx::macos_click_pulse
