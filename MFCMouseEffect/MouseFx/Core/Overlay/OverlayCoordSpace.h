#pragma once

#include <cstdint>

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

void SetOverlayWindowHandle(uintptr_t windowHandle);
void ClearOverlayWindowHandle();
void SetOverlayOriginOverride(int x, int y);
void ClearOverlayOriginOverride();
ScreenPoint GetOverlayOrigin();
ScreenPoint ScreenToOverlayPoint(const ScreenPoint& screenPt);

} // namespace mousefx
