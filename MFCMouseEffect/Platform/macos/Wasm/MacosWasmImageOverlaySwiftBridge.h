#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_wasm_image_overlay_create_v1(
    double frameX,
    double frameY,
    double frameSize,
    const char* imagePathUtf8,
    unsigned int tintArgb,
    int applyTint,
    double alphaScale,
    double durationSec,
    double rotationRad,
    double motionDx,
    double motionDy);

void mfx_macos_wasm_image_overlay_show_v1(void* windowHandle);

#ifdef __cplusplus
}
#endif
