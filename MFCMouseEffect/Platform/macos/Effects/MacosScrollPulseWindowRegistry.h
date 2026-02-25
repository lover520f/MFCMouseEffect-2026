#pragma once

namespace mousefx::macos_scroll_pulse {

void RegisterScrollPulseWindow(void* windowHandle);
bool TakeScrollPulseWindow(void* windowHandle);
void CloseAllScrollPulseWindowsNow();

} // namespace mousefx::macos_scroll_pulse
