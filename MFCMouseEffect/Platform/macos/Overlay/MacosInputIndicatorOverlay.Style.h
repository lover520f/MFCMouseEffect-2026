#pragma once

#include <string>

namespace mousefx::macos_input_indicator_style {

#if defined(__APPLE__)
void* CreatePanel(int sizePx);
void ReleasePanel(void* panelHandle);
void HidePanel(void* panelHandle);
void ApplyPanelPresentation(void* panelHandle, int x, int y, int sizePx, int durationMs, const std::string& text);
#endif

} // namespace mousefx::macos_input_indicator_style
