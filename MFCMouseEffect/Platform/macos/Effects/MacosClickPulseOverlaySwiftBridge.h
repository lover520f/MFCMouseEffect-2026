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
    unsigned int glowArgb,
    double baseOpacity,
    double animationDurationSec,
    double startRadiusPx,
    double endRadiusPx,
    double strokeWidthPx,
    const char* textLabelUtf8,
    double textFontSizePx,
    double textFloatDistancePx,
    const char* textFontFamilyUtf8,
    int textEmoji);

#ifdef __cplusplus
}
#endif
