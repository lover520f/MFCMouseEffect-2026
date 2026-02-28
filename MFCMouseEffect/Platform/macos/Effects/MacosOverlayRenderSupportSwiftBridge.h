#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_overlay_create_window_v1(double x, double y, double width, double height);
void mfx_macos_overlay_release_window_v1(void* windowHandle);
double mfx_macos_overlay_resolve_content_scale_v1(int x, int y);
void mfx_macos_overlay_apply_content_scale_v1(void* contentHandle, int x, int y);

#ifdef __cplusplus
}
#endif
