#pragma once

#include <algorithm>
#include <cstdint>

namespace mousefx {

// Clamp an integer value to [lo, hi].
inline int ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Clamp a float value to [lo, hi].
inline float ClampFloat(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

} // namespace mousefx
