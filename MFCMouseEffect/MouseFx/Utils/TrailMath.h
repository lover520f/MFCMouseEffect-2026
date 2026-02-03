#pragma once

#include <algorithm>
#include <cstdint>

namespace mousefx {
namespace trail_math {

inline float Clamp01(float x) {
    return std::max(0.0f, std::min(1.0f, x));
}

inline float Clamp(float x, float minV, float maxV) {
    if (x < minV) return minV;
    if (x > maxV) return maxV;
    return x;
}

inline float Smoothstep01(float x) {
    x = Clamp01(x);
    return x * x * (3.0f - 2.0f * x);
}

inline float IdleFadeFactor(uint64_t nowMs, uint64_t lastPointMs, int fadeStartMs, int fadeEndMs) {
    if (fadeEndMs <= fadeStartMs) return 1.0f;
    uint64_t age = (nowMs >= lastPointMs) ? (nowMs - lastPointMs) : 0;
    if ((int)age <= fadeStartMs) return 1.0f;
    float t = ((float)age - (float)fadeStartMs) / ((float)fadeEndMs - (float)fadeStartMs);
    return 1.0f - Smoothstep01(t);
}

} // namespace trail_math
} // namespace mousefx
