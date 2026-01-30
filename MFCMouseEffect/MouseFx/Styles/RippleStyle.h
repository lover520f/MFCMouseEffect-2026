#pragma once

#include <cstdint>

namespace mousefx {

// Simple ARGB color (0xAARRGGBB).
struct Argb {
    uint32_t value{0};
};

struct RippleStyle {
    // Overall animation timing.
    uint32_t durationMs = 320;

    // Base sizes in pixels (window will be square).
    int windowSize = 220;
    float startRadius = 14.0f;
    float endRadius = 60.0f;

    // Visuals.
    float strokeWidth = 2.0f;
    Argb fill = {0x594FC3F7};   // translucent fill
    Argb stroke = {0xFF0288D1}; // solid stroke
    Argb glow = {0x660288D1};   // glow color (shadow)
};

} // namespace mousefx

