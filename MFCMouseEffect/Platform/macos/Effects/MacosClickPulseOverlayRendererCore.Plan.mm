#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
ClickPulseRenderPlan BuildClickPulseRenderPlan(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const macos_effect_profile::ClickRenderProfile& profile) {
    ClickPulseRenderPlan plan{};
    plan.normalizedType = NormalizeClickType(effectType);
    plan.textMode = (plan.normalizedType == "text");
    plan.starMode = (plan.normalizedType == "star");

    plan.size = plan.textMode
        ? static_cast<CGFloat>(profile.textSizePx)
        : static_cast<CGFloat>(profile.normalSizePx);
    plan.inset = plan.textMode ? 12.0 : 18.0;
    plan.frame = NSMakeRect(
        overlayPt.x - plan.size * 0.5,
        overlayPt.y - plan.size * 0.5,
        plan.size,
        plan.size);
    plan.animationDuration = plan.textMode ? profile.textDurationSec : profile.normalDurationSec;
    return plan;
}

int64_t ComputeClickPulseCloseDelayNs(
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile) {
    return static_cast<int64_t>(
               static_cast<int>(plan.animationDuration * 1000.0) + profile.closePaddingMs) *
           NSEC_PER_MSEC;
}

#endif

} // namespace mousefx::macos_click_pulse
