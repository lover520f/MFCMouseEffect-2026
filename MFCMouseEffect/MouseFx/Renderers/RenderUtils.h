#pragma once

#define NOMINMAX
#include <algorithm>
#include <cmath>
#include <gdiplus.h>
#include "../Styles/RippleStyle.h"

namespace mousefx {
namespace render_utils {

    inline float Clamp01(float v) { 
        return (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v); 
    }

    inline int ClampInt(int v, int lo, int hi) { 
        if (v < lo) return lo; 
        if (v > hi) return hi; 
        return v; 
    }

    inline BYTE ClampByte(int v) { 
        return (BYTE)ClampInt(v, 0, 255); 
    }
    
    inline Gdiplus::Color ToGdiPlus(const Argb& c) {
        return Gdiplus::Color((BYTE)((c.value >> 24) & 0xFF), 
                              (BYTE)((c.value >> 16) & 0xFF), 
                              (BYTE)((c.value >> 8) & 0xFF), 
                              (BYTE)(c.value & 0xFF));
    }

    inline float fnmod(float x, float y) {
        return x - y * floor(x / y);
    }

} // namespace render_utils
} // namespace mousefx
