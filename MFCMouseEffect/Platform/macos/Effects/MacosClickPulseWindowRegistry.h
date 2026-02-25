#pragma once

#include <cstddef>

namespace mousefx::macos_click_pulse {

void RegisterClickPulseWindow(void* windowHandle);
bool TakeClickPulseWindow(void* windowHandle);
void CloseAllClickPulseWindowsNow();
size_t GetActiveClickPulseWindowCount();

} // namespace mousefx::macos_click_pulse
