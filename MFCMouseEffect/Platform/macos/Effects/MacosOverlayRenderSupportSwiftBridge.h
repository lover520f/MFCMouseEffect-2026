#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_overlay_create_window_v1(double x, double y, double width, double height);
void mfx_macos_overlay_release_window_v1(void* windowHandle);
void mfx_macos_overlay_show_window_v1(void* windowHandle);
int mfx_macos_overlay_resolve_screen_frame_v1(
    int x,
    int y,
    double* outX,
    double* outY,
    double* outWidth,
    double* outHeight);
double mfx_macos_overlay_resolve_content_scale_v1(int x, int y);
void mfx_macos_overlay_apply_content_scale_v1(void* contentHandle, int x, int y);

#ifdef __cplusplus
}
#endif
