#pragma once

#include "../RenderUtils.h"

#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mousefx {
namespace neon3d {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline Vec3 RotX(const Vec3& v, float angle) {
    const float c = cos(angle), s = sin(angle);
    return { v.x, v.y * c - v.z * s, v.y * s + v.z * c };
}

inline Vec3 RotY(const Vec3& v, float angle) {
    const float c = cos(angle), s = sin(angle);
    return { v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
}

inline Vec3 RotZ(const Vec3& v, float angle) {
    const float c = cos(angle), s = sin(angle);
    return { v.x * c - v.y * s, v.x * s + v.y * c, v.z };
}

inline Gdiplus::PointF Project(const Vec3& v, float cx, float cy, float dist = 520.0f) {
    const float scale = dist / (dist + v.z);
    return Gdiplus::PointF(cx + v.x * scale, cy + v.y * scale);
}

inline Gdiplus::PointF Lerp(const Gdiplus::PointF& a, const Gdiplus::PointF& b, float t) {
    return Gdiplus::PointF(a.X + (b.X - a.X) * t, a.Y + (b.Y - a.Y) * t);
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float Smoothstep(float edge0, float edge1, float x) {
    using namespace mousefx::render_utils;
    const float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

inline float EaseOutBack(float x) {
    using namespace mousefx::render_utils;
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    const float t = Clamp01(x) - 1.0f;
    return 1.0f + c3 * t * t * t + c1 * t * t;
}

inline Gdiplus::PointF Dir(float aRad) {
    return Gdiplus::PointF((float)cos(aRad), (float)sin(aRad));
}

inline Gdiplus::PointF Polar(float cx, float cy, float r, float aRad) {
    const Gdiplus::PointF d = Dir(aRad);
    return Gdiplus::PointF(cx + d.X * r, cy + d.Y * r);
}

inline float RadToDeg(float rad) {
    return rad * (180.0f / 3.1415926f);
}

inline float DegToRad(float deg) {
    return deg * (3.1415926f / 180.0f);
}

inline Gdiplus::Color MulAlpha(const Gdiplus::Color& c, float a01) {
    using namespace mousefx::render_utils;
    const int a = ClampByte((int)std::round((float)c.GetA() * Clamp01(a01)));
    return Gdiplus::Color((BYTE)a, c.GetR(), c.GetG(), c.GetB());
}

inline Gdiplus::Color LerpRgb(const Gdiplus::Color& a, const Gdiplus::Color& b, float t) {
    using namespace mousefx::render_utils;
    const float tt = Clamp01(t);
    const int r = (int)std::round((float)a.GetR() + ((float)b.GetR() - (float)a.GetR()) * tt);
    const int g = (int)std::round((float)a.GetG() + ((float)b.GetG() - (float)a.GetG()) * tt);
    const int bl = (int)std::round((float)a.GetB() + ((float)b.GetB() - (float)a.GetB()) * tt);
    return Gdiplus::Color(255, (BYTE)ClampByte(r), (BYTE)ClampByte(g), (BYTE)ClampByte(bl));
}

class StableHash final {
public:
    explicit StableHash(uint32_t seed) : seed_(seed) {}

    float Hash01(uint32_t v) const {
        v ^= seed_;
        v ^= v >> 16;
        v *= 0x7feb352dU;
        v ^= v >> 15;
        v *= 0x846ca68bU;
        v ^= v >> 16;
        return (float)(v & 0x00FFFFFFu) / (float)0x01000000u;
    }

private:
    uint32_t seed_ = 0;
};

class LcgRng final {
public:
    explicit LcgRng(uint32_t seed) : state_(seed ? seed : 1u) {}

    uint32_t NextU32() {
        state_ = state_ * 1664525u + 1013904223u;
        return state_;
    }

    float Next01() {
        return (float)(NextU32() >> 8) / (float)0x01000000u;
    }

private:
    uint32_t state_ = 1u;
};

} // namespace neon3d
} // namespace mousefx
