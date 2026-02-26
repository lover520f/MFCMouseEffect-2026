#pragma once

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

namespace mousefx::macos_overlay_coord_conversion {

bool TryConvertQuartzToCocoa(const ScreenPoint& input, ScreenPoint* output);

} // namespace mousefx::macos_overlay_coord_conversion
