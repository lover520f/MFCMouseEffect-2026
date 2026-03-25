#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"

#include "Platform/macos/Overlay/MacosCursorDecorationSwiftBridge.h"
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

void* CreateDecorationPanel() {
    return mfx_macos_cursor_decoration_panel_create_v1();
}

void ReleaseDecorationPanel(void* panelHandle) {
    mfx_macos_cursor_decoration_panel_release_v1(panelHandle);
}

void HideDecorationPanel(void* panelHandle) {
    mfx_macos_cursor_decoration_panel_hide_v1(panelHandle);
}

void ApplyDecorationPanelPresentation(
    void* panelHandle,
    int x,
    int y,
    const std::string& pluginId,
    const std::string& colorHex,
    int sizePx,
    int alphaPercent) {
    const char* pluginIdUtf8 = pluginId.empty() ? "focus_ring" : pluginId.c_str();
    const char* colorUtf8 = colorHex.empty() ? "#ff5a5a" : colorHex.c_str();
    mfx_macos_cursor_decoration_panel_present_v1(
        panelHandle,
        x,
        y,
        pluginIdUtf8,
        colorUtf8,
        sizePx,
        alphaPercent);
}

#endif

} // namespace mousefx::macos_input_indicator_style
