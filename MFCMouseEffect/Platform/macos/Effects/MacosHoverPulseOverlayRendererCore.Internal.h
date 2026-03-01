#pragma once

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
struct HoverPulseRenderPlan {
    HoverEffectRenderCommand command{};
    CGFloat size = 0;
    CGRect frame = CGRectZero;
    CFTimeInterval breatheDurationSec = 0;
    CFTimeInterval tubesSpinDurationSec = 0;
};

HoverPulseRenderPlan BuildHoverPulseRenderPlan(const HoverEffectRenderCommand& command);
#endif

} // namespace mousefx::macos_hover_pulse
