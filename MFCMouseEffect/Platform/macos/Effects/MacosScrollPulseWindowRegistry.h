#pragma once

#include <cstddef>

namespace mousefx::macos_scroll_pulse {

void RegisterScrollPulseWindow(void* windowHandle);
bool TakeScrollPulseWindow(void* windowHandle);
void CloseAllScrollPulseWindowsNow();
size_t GetActiveScrollPulseWindowCount();

} // namespace mousefx::macos_scroll_pulse
