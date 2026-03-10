#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"

#include "Platform/macos/Overlay/MacosInputIndicatorSwiftBridge.h"

namespace mousefx::macos_input_indicator_style {

#if defined(__APPLE__)

void* CreatePanel(int sizePx) {
    return mfx_macos_input_indicator_panel_create_v1(sizePx);
}

void ReleasePanel(void* panelHandle) {
    mfx_macos_input_indicator_panel_release_v1(panelHandle);
}

void HidePanel(void* panelHandle) {
    mfx_macos_input_indicator_panel_hide_v1(panelHandle);
}

void ApplyPanelPresentation(void* panelHandle, int x, int y, int sizePx, int durationMs, const std::string& text) {
    const char* textUtf8 = text.empty() ? "" : text.c_str();
    mfx_macos_input_indicator_panel_present_v2(panelHandle, x, y, sizePx, textUtf8, durationMs);
}

#endif

} // namespace mousefx::macos_input_indicator_style
