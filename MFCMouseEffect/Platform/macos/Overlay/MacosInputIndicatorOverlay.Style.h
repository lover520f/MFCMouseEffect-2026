#pragma once

#include <string>

namespace mousefx::macos_input_indicator_style {

#if defined(__APPLE__)
void* CreatePanel(int sizePx);
void ReleasePanel(void* panelHandle);
void HidePanel(void* panelHandle);
void ApplyPanelPresentation(void* panelHandle, int x, int y, int sizePx, int durationMs, const std::string& text);
void* CreateDecorationPanel();
void ReleaseDecorationPanel(void* panelHandle);
void HideDecorationPanel(void* panelHandle);
void ApplyDecorationPanelPresentation(
    void* panelHandle,
    int x,
    int y,
    const std::string& pluginId,
    const std::string& colorHex,
    int sizePx,
    int alphaPercent);
#endif

} // namespace mousefx::macos_input_indicator_style
