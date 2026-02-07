#pragma once

#include "Neon3DMath.h"

#include <gdiplus.h>
#include <algorithm>
#include <cmath>

namespace mousefx {
namespace neon3d {

struct NeonPalette {
    Gdiplus::Color cyan;
    Gdiplus::Color purple;
    Gdiplus::Color mint;
};

inline float WrapHueDeg(float hDeg) {
    float h = (float)std::fmod(hDeg, 360.0f);
    if (h < 0.0f) h += 360.0f;
    return h;
}

inline void RgbToHsl(BYTE r, BYTE g, BYTE b, float& hDeg, float& s, float& l) {
    const float rf = (float)r / 255.0f;
    const float gf = (float)g / 255.0f;
    const float bf = (float)b / 255.0f;

    const float maxc = std::max(rf, std::max(gf, bf));
    const float minc = std::min(rf, std::min(gf, bf));
    const float d = maxc - minc;

    l = (maxc + minc) * 0.5f;

    if (d <= 1e-6f) {
        hDeg = 0.0f;
        s = 0.0f;
        return;
    }

    s = (l > 0.5f) ? (d / (2.0f - maxc - minc)) : (d / (maxc + minc));

    float h = 0.0f;
    if (maxc == rf) {
        h = (gf - bf) / d + (gf < bf ? 6.0f : 0.0f);
    } else if (maxc == gf) {
        h = (bf - rf) / d + 2.0f;
    } else {
        h = (rf - gf) / d + 4.0f;
    }
    hDeg = WrapHueDeg(h * 60.0f);
}

inline float Hue2Rgb(float p, float q, float t) {
    float tt = t;
    if (tt < 0.0f) tt += 1.0f;
    if (tt > 1.0f) tt -= 1.0f;
    if (tt < 1.0f / 6.0f) return p + (q - p) * 6.0f * tt;
    if (tt < 1.0f / 2.0f) return q;
    if (tt < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - tt) * 6.0f;
    return p;
}

inline Gdiplus::Color HslToRgb(float hDeg, float s, float l, BYTE a = 255) {
    using namespace mousefx::render_utils;
    const float hh = WrapHueDeg(hDeg) / 360.0f;
    const float ss = Clamp01(s);
    const float ll = Clamp01(l);

    float rf = ll, gf = ll, bf = ll;
    if (ss > 1e-6f) {
        const float q = (ll < 0.5f) ? (ll * (1.0f + ss)) : (ll + ss - ll * ss);
        const float p = 2.0f * ll - q;
        rf = Hue2Rgb(p, q, hh + 1.0f / 3.0f);
        gf = Hue2Rgb(p, q, hh);
        bf = Hue2Rgb(p, q, hh - 1.0f / 3.0f);
    }

    const BYTE r = ClampByte((int)std::lround(rf * 255.0f));
    const BYTE g = ClampByte((int)std::lround(gf * 255.0f));
    const BYTE b = ClampByte((int)std::lround(bf * 255.0f));
    return Gdiplus::Color(a, r, g, b);
}

inline NeonPalette MakeNeonPalette(const Gdiplus::Color& primaryRgb) {
    float h = 0.0f, s = 0.0f, l = 0.0f;
    RgbToHsl(primaryRgb.GetR(), primaryRgb.GetG(), primaryRgb.GetB(), h, s, l);

    // If the input is near-gray, fall back to a cyan-centered palette (concept default).
    if (s < 0.08f) {
        h = 190.0f;
        s = 0.92f;
        l = 0.60f;
    }

    // Concept alignment: keep the overall palette in a cool cyan/purple range.
    // If a theme provides a warm primary (yellow/orange/red), force the base hue back toward cyan
    // so the HUD does not become "golden lightning".
    //
    // Roughly allow: 150..260 degrees (cyan/blue region).
    if (s >= 0.18f) {
        if (h < 150.0f || h > 260.0f) {
            h = 190.0f;
        }
    }

    const float sat = std::max(0.72f, s);
    const float light = std::max(0.55f, std::min(0.68f, l));

    NeonPalette pal{};
    pal.cyan = HslToRgb(h, sat, std::min(0.72f, light + 0.06f), 255);
    pal.purple = HslToRgb(h + 90.0f, std::min(1.0f, sat + 0.10f), std::min(0.70f, light + 0.02f), 255);
    pal.mint = HslToRgb(h - 40.0f, sat, std::min(0.72f, light + 0.08f), 255);
    return pal;
}

} // namespace neon3d
} // namespace mousefx
