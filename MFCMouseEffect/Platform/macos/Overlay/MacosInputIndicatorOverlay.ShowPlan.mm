#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx::macos_input_indicator_detail {

IndicatorShowPlan BuildIndicatorShowPlan(const InputIndicatorConfig& cfg, ScreenPoint point) {
    IndicatorShowPlan plan{};
    plan.sizePx = macos_input_indicator::ClampInt(cfg.sizePx, 28, 220);
    plan.durationMs = macos_input_indicator::ClampInt(cfg.durationMs, 80, 5000);

    const bool absolute = (ToLowerAscii(TrimAscii(cfg.positionMode)) == "absolute");
    const ScreenPoint overlayPoint = absolute ? point : ScreenToOverlayPoint(point);
    plan.x = absolute ? cfg.absoluteX : (overlayPoint.x + cfg.offsetX);
    plan.y = absolute ? cfg.absoluteY : (overlayPoint.y + cfg.offsetY);
    return plan;
}

} // namespace mousefx::macos_input_indicator_detail
