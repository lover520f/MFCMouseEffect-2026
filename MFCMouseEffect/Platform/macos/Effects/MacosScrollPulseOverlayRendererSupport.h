#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#if defined(__APPLE__)
@class CAShapeLayer;
#import <CoreGraphics/CoreGraphics.h>
#endif

namespace mousefx::macos_scroll_pulse::support {

int ResolveStrengthLevel(int delta);
CGRect BuildBodyRect(CGFloat size, bool horizontal, int strengthLevel, double intensity);
CFTimeInterval BuildPulseDuration(
    const macos_effect_profile::ScrollRenderProfile& profile,
    int strengthLevel,
    CGFloat overlaySize);
int BuildCloseAfterMs(
    const macos_effect_profile::ScrollRenderProfile& profile,
    CFTimeInterval duration);

#if defined(__APPLE__)
CAShapeLayer* CreateBodyLayer(
    CGRect bounds,
    CGRect bodyRect,
    bool horizontal,
    int delta,
    double baseOpacity,
    const macos_effect_profile::ScrollRenderProfile& profile);

CAShapeLayer* CreateArrowLayer(
    CGRect bounds,
    CGRect bodyRect,
    bool horizontal,
    int delta,
    double baseOpacity,
    const macos_effect_profile::ScrollRenderProfile& profile);
#endif

} // namespace mousefx::macos_scroll_pulse::support
