#pragma once

#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"

namespace mousefx::macos_input_indicator_detail {

struct IndicatorShowPlan final {
    int sizePx = 72;
    int durationMs = 1400;
    int x = 0;
    int y = 0;
};

IndicatorShowPlan BuildIndicatorShowPlan(const InputIndicatorConfig& cfg, ScreenPoint point);

} // namespace mousefx::macos_input_indicator_detail
