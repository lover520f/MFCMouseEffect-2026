#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#ifdef __OBJC__
@class CAShapeLayer;
#else
struct objc_object;
using CAShapeLayer = objc_object;
#endif
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
    double baseOpacity,
    uint32_t fillArgb,
    uint32_t strokeArgb);

CAShapeLayer* CreateArrowLayer(
    CGRect bounds,
    CGRect bodyRect,
    bool horizontal,
    int delta,
    double baseOpacity,
    uint32_t strokeArgb);
#endif

} // namespace mousefx::macos_scroll_pulse::support
