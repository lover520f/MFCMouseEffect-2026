#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_input_indicator_panel_create_v1(int sizePx);
void mfx_macos_input_indicator_panel_release_v1(void* panelHandle);
void mfx_macos_input_indicator_panel_hide_v1(void* panelHandle);
void mfx_macos_input_indicator_panel_present_v1(
    void* panelHandle,
    int x,
    int y,
    int sizePx,
    const char* textUtf8);
void mfx_macos_input_indicator_panel_present_v2(
    void* panelHandle,
    int x,
    int y,
    int sizePx,
    const char* textUtf8,
    int durationMs);

#ifdef __cplusplus
}
#endif
