#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_line_trail_create_v1(void);
void mfx_macos_line_trail_release_v1(void* handle);
void mfx_macos_line_trail_reset_v1(void* handle);
void mfx_macos_line_trail_update_v1(
    void* handle,
    int overlayX,
    int overlayY,
    int durationMs,
    float lineWidth,
    unsigned int strokeArgb,
    int idleFadeStartMs,
    int idleFadeEndMs);

#ifdef __cplusplus
}
#endif
