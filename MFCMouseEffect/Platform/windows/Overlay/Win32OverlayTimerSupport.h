#pragma once

#include <windows.h>

namespace mousefx::win32_overlay_timer_support {

void SetOverlayTargetFps(int targetFps);
int ResolveTimerIntervalMsForScreenPoint(int x, int y);
int ResolveTimerIntervalMsForCursor();
int ResolveTimerIntervalMsForWindow(HWND hwnd);
int ResolveTimerIntervalMsForVirtualScreen();

} // namespace mousefx::win32_overlay_timer_support

