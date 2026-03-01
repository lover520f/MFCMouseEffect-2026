#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_hold_pulse_overlay_create_v1(
    double frameX,
    double frameY,
    double frameSize,
    int overlayX,
    int overlayY,
    unsigned int baseStrokeArgb,
    int holdStyleCode,
    double baseOpacity,
    double breatheDurationSec,
    double rotateDurationSec,
    double rotateDurationFastSec,
    void** ringLayerOut,
    void** accentLayerOut);

void mfx_macos_hold_pulse_overlay_update_v1(
    void* windowHandle,
    void* ringLayerHandle,
    void* accentLayerHandle,
    double frameOriginX,
    double frameOriginY,
    int overlayX,
    int overlayY,
    double baseOpacity,
    unsigned int progressFullMs,
    unsigned int holdMs);

#ifdef __cplusplus
}
#endif

