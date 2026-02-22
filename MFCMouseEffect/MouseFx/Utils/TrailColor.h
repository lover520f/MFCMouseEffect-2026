#pragma once

#include <gdiplus.h>
#include <cstdint>

namespace mousefx {
namespace trail_color {

inline Gdiplus::Color HslToRgbColor(float h, float s, float l, uint8_t alpha) {
    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    float p = 2.0f * l - q;

    float tr = h / 360.0f + 1.0f / 3.0f;
    float tg = h / 360.0f;
    float tb = h / 360.0f - 1.0f / 3.0f;

    return Gdiplus::Color(
        alpha,
        static_cast<uint8_t>(hue2rgb(p, q, tr) * 255.0f),
        static_cast<uint8_t>(hue2rgb(p, q, tg) * 255.0f),
        static_cast<uint8_t>(hue2rgb(p, q, tb) * 255.0f)
    );
}

} // namespace trail_color
} // namespace mousefx

