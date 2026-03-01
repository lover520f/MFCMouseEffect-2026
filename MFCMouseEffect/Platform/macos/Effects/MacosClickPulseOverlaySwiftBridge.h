#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mfx_macos_click_pulse_overlay_create_v1(
    double frameX,
    double frameY,
    double frameSize,
    double inset,
    int overlayX,
    int overlayY,
    const char* normalizedTypeUtf8,
    unsigned int fillArgb,
    unsigned int strokeArgb,
    double baseOpacity,
    double animationDurationSec,
    const char* textLabelUtf8);

#ifdef __cplusplus
}
#endif

