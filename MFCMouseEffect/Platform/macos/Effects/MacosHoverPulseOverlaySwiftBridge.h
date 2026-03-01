#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_hover_pulse_overlay_create_v1(
    double frameX,
    double frameY,
    double frameSize,
    int overlayX,
    int overlayY,
    unsigned int glowFillArgb,
    unsigned int glowStrokeArgb,
    unsigned int tubesStrokeArgb,
    double baseOpacity,
    double breatheDurationSec,
    double tubesSpinDurationSec,
    int tubesMode);

#ifdef __cplusplus
}
#endif
