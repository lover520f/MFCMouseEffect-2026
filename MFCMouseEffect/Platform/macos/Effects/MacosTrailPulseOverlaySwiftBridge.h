#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_trail_pulse_overlay_create_v1(
    double frameX,
    double frameY,
    double frameWidth,
    double frameHeight,
    int overlayX,
    int overlayY,
    const char* normalizedTypeUtf8,
    int tubesMode,
    int particleMode,
    int glowMode,
    double deltaX,
    double deltaY,
    double intensity,
    int sizePx,
    double lineWidthPx,
    double durationSec,
    double baseOpacity,
    unsigned int fillArgb,
    unsigned int strokeArgb);

#ifdef __cplusplus
}
#endif
