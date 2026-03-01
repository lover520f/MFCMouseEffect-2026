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
int mfx_macos_line_trail_is_active_v1(void* handle);
int mfx_macos_line_trail_point_count_v1(void* handle);

#ifdef __cplusplus
}
#endif
