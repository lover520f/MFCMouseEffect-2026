#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_cursor_decoration_panel_create_v1(void);
void mfx_macos_cursor_decoration_panel_release_v1(void* panelHandle);
void mfx_macos_cursor_decoration_panel_hide_v1(void* panelHandle);
void mfx_macos_cursor_decoration_panel_present_v1(
    void* panelHandle,
    int x,
    int y,
    const char* pluginIdUtf8,
    const char* colorHexUtf8,
    int sizePx,
    int alphaPercent);

#ifdef __cplusplus
}
#endif
