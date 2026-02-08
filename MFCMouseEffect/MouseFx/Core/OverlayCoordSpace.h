#pragma once

#include <windows.h>

namespace mousefx {

void SetOverlayWindowHandle(HWND hwnd);
void ClearOverlayWindowHandle();
void SetOverlayOriginOverride(int x, int y);
void ClearOverlayOriginOverride();
POINT GetOverlayOrigin();
POINT ScreenToOverlayPoint(const POINT& screenPt);

} // namespace mousefx
