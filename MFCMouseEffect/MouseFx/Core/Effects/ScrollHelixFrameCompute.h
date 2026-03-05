#pragma once
/// ScrollHelixFrameCompute.h
/// Platform-agnostic per-frame helix computation.
/// Produces a sorted list of line segments + head highlights
/// that any renderer (GDI+, CoreGraphics, etc.) can draw directly.

#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>

namespace mousefx {

// ── Helix frame data (platform-independent) ───────────────────

struct ScrollHelixSegment {
    float z;
    float x1, y1, x2, y2;
    float width;
    float r, g, b, a;  // pre-mixed 0-1 color
};

struct ScrollHelixHead {
    float x, y, alpha;
    float strokeR, strokeG, strokeB;
};

struct ScrollHelixFrameData {
    std::vector<ScrollHelixSegment> segments; // back-to-front sorted
    std::vector<ScrollHelixHead> heads;
};

// ── Helix math helpers ────────────────────────────────────────

namespace helix_math {

inline float Clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

inline float Smoothstep(float a, float b, float x) {
    if (b <= a) return x >= b ? 1.0f : 0.0f;
    float t = (x - a) / (b - a);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

inline float Mix(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

} // namespace helix_math

// ── Frame computation ─────────────────────────────────────────

inline ScrollHelixFrameData ComputeHelixFrame(
    float t,
    uint64_t elapsedMs,
    int sizePx,
    float directionRad,
    float intensity,
    float startRadius,
    float endRadius,
    float strokeWidth,
    float strokeR, float strokeG, float strokeB,
    float glowR, float glowG, float glowB
) {
    using namespace helix_math;
    ScrollHelixFrameData frame;

    const float tn = Clamp01(t);
    const float fadeOut = 1.0f - Smoothstep(0.72f, 1.0f, tn);
    const float alphaBase = fadeOut * Clamp01(intensity);
    if (alphaBase <= 0.001f) return frame;

    const float cx = sizePx / 2.0f;
    const float cy = sizePx / 2.0f;
    const float dir = directionRad + 3.1415926f;
    const float dx = std::cos(dir);
    const float dy = std::sin(dir);
    const float px = -dy;
    const float py = dx;

    const float seconds = static_cast<float>(elapsedMs) / 1000.0f;
    const float phase = seconds * 1.55f * 3.1415926f;

    constexpr int kSegments = 44;
    const float length = (endRadius - startRadius) * 2.05f;
    const float radius = strokeWidth * 2.2f + 6.2f;
    const float camera = radius * 5.2f;
    const float depthAmp = radius * 0.95f;

    struct Point3D { float x, y, z, w, a, u; };

    auto getPoint = [&](int i, float offsetRad) -> Point3D {
        const float u = static_cast<float>(i) / static_cast<float>(kSegments);
        const float dist = (u - 0.5f) * length + (1.0f - fadeOut) * (length * 0.14f);
        const float turns = 1.72f;
        const float angle = u * turns * 6.2831853f - phase + offsetRad;
        const float localY = std::sin(angle) * radius;
        const float localZ = std::cos(angle) * depthAmp;
        const float persp = camera / (camera + localZ + depthAmp);
        const float sx = cx + dx * dist + px * (localY * persp);
        const float sy = cy + dy * dist + py * (localY * persp);
        const float taper = 0.30f + 0.70f * u;
        const float depthLight = 0.52f + 0.48f * ((localZ / depthAmp) * 0.5f + 0.5f);
        const float headBoost = 0.62f + 0.38f * Smoothstep(0.55f, 1.0f, u);
        return { sx, sy, localZ / depthAmp,
                 (0.62f + 0.38f * persp) * taper,
                 taper * depthLight * headBoost, u };
    };

    std::vector<Point3D> strandA, strandB;
    strandA.reserve(kSegments);
    strandB.reserve(kSegments);
    for (int i = 0; i < kSegments; ++i) {
        strandA.push_back(getPoint(i, 0.0f));
        strandB.push_back(getPoint(i, 3.1415926f));
    }

    frame.segments.reserve(static_cast<size_t>(kSegments) * 3);

    for (int i = 0; i < kSegments - 1; ++i) {
        const auto& p1 = strandA[static_cast<size_t>(i)];
        const auto& p2 = strandA[static_cast<size_t>(i) + 1];
        float avgZ = (p1.z + p2.z) * 0.5f;
        float avgW = (p1.w + p2.w) * 0.5f * (strokeWidth + 0.9f);
        float avgA = (p1.a + p2.a) * 0.5f * alphaBase;
        float avgU = (p1.u + p2.u) * 0.5f;
        float sm = 0.25f + 0.45f * avgU;
        frame.segments.push_back({
            avgZ, p1.x, p1.y, p2.x, p2.y, avgW,
            Mix(strokeR, 1.0f, sm), Mix(strokeG, 1.0f, sm), Mix(strokeB, 1.0f, sm), avgA
        });

        const auto& q1 = strandB[static_cast<size_t>(i)];
        const auto& q2 = strandB[static_cast<size_t>(i) + 1];
        float avgZ2 = (q1.z + q2.z) * 0.5f;
        float avgW2 = (q1.w + q2.w) * 0.5f * (strokeWidth + 0.9f);
        float avgA2 = (q1.a + q2.a) * 0.5f * alphaBase;
        float avgU2 = (q1.u + q2.u) * 0.5f;
        float sm2 = 0.20f + 0.32f * avgU2;
        frame.segments.push_back({
            avgZ2, q1.x, q1.y, q2.x, q2.y, avgW2,
            Mix(glowR, 1.0f, sm2), Mix(glowG, 1.0f, sm2), Mix(glowB, 1.0f, sm2), avgA2
        });

        // Keep helix strands independent (no ladder-style crossbars).
        // This preserves the expected "double helix" visual semantics on both
        // Windows and macOS because both platforms consume this shared frame data.
    }

    std::sort(frame.segments.begin(), frame.segments.end(),
              [](const ScrollHelixSegment& a, const ScrollHelixSegment& b) { return a.z < b.z; });

    for (float offset : {0.0f, 3.1415926f}) {
        auto head = getPoint(kSegments - 1, offset);
        frame.heads.push_back({ head.x, head.y, alphaBase, strokeR, strokeG, strokeB });
    }

    return frame;
}

} // namespace mousefx
