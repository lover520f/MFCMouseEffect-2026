#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_text_panel_create_v1(
    const char* textUtf8,
    double panelSize,
    double fontSize,
    unsigned int argb,
    const char* fontFamilyUtf8,
    int emojiText);
void mfx_macos_text_panel_release_v1(void* panelHandle);
void mfx_macos_text_panel_show_v1(void* panelHandle);
void mfx_macos_text_panel_set_frame_v1(void* panelHandle, double x, double y, double panelSize);
void mfx_macos_text_panel_apply_style_v1(
    void* panelHandle,
    double fontSize,
    unsigned int argb,
    double alphaScale,
    const char* fontFamilyUtf8,
    int emojiText);

#ifdef __cplusplus
}
#endif
