#pragma once

#include "Neon3DMath.h"

#include <gdiplus.h>
#include <vector>

namespace mousefx {
namespace neon3d {

inline Gdiplus::PointF Bezier(const Gdiplus::PointF& p0, const Gdiplus::PointF& p1, const Gdiplus::PointF& p2, const Gdiplus::PointF& p3, float t) {
    const float u = 1.0f - t;
    const float tt = t * t;
    const float uu = u * u;
    const float uuu = uu * u;
    const float ttt = tt * t;
    return Gdiplus::PointF(
        uuu * p0.X + 3.0f * uu * t * p1.X + 3.0f * u * tt * p2.X + ttt * p3.X,
        uuu * p0.Y + 3.0f * uu * t * p1.Y + 3.0f * u * tt * p2.Y + ttt * p3.Y
    );
}

inline std::vector<Gdiplus::PointF> SampleBezier(const Gdiplus::PointF& p0, const Gdiplus::PointF& p1,
                                                 const Gdiplus::PointF& p2, const Gdiplus::PointF& p3, int segments) {
    if (segments < 2) segments = 2;
    std::vector<Gdiplus::PointF> pts;
    pts.reserve((size_t)segments + 1);
    for (int i = 0; i <= segments; ++i) {
        const float t = (float)i / (float)segments;
        pts.push_back(Bezier(p0, p1, p2, p3, t));
    }
    return pts;
}

inline Gdiplus::PointF PolyNormal(const std::vector<Gdiplus::PointF>& pts, int i) {
    const int n = (int)pts.size();
    if (n < 2) return Gdiplus::PointF(0, 0);
    const int i0 = (i <= 0) ? 0 : (i - 1);
    const int i1 = (i >= n - 1) ? (n - 1) : (i + 1);
    const float dx = pts[i1].X - pts[i0].X;
    const float dy = pts[i1].Y - pts[i0].Y;
    float nx = -dy;
    float ny = dx;
    const float len = (float)std::sqrt(nx * nx + ny * ny);
    if (len <= 0.0001f) return Gdiplus::PointF(0, 0);
    nx /= len;
    ny /= len;
    return Gdiplus::PointF(nx, ny);
}

inline std::vector<Gdiplus::PointF> JitterPolyline(const std::vector<Gdiplus::PointF>& pts, float timeSec, float progress01,
                                                   const float* seeds, int seedsCount, float ampBase, float ampScale, float freq) {
    using namespace mousefx::render_utils;
    std::vector<Gdiplus::PointF> out;
    out.reserve(pts.size());
    const int n = (int)pts.size();
    for (int i = 0; i < n; ++i) {
        const float u = (n <= 1) ? 0.0f : (float)i / (float)(n - 1);
        const float wob = (float)sin(timeSec * freq + (seedsCount > 0 ? seeds[i % seedsCount] : 0.0f) + u * 12.0f);
        const float amp = (ampBase + ampScale * u) * (0.6f + 0.7f * Clamp01(progress01));
        const Gdiplus::PointF nn = PolyNormal(pts, i);
        out.push_back(Gdiplus::PointF(pts[i].X + nn.X * wob * amp, pts[i].Y + nn.Y * wob * amp));
    }
    return out;
}

inline std::vector<Gdiplus::PointF> SmoothPolyline(const std::vector<Gdiplus::PointF>& pts, int iterations) {
    if (iterations <= 0 || pts.size() < 3) return pts;
    std::vector<Gdiplus::PointF> cur = pts;
    std::vector<Gdiplus::PointF> next = pts;
    for (int it = 0; it < iterations; ++it) {
        next[0] = cur[0];
        for (size_t i = 1; i + 1 < cur.size(); ++i) {
            const Gdiplus::PointF& p0 = cur[i - 1];
            const Gdiplus::PointF& p1 = cur[i];
            const Gdiplus::PointF& p2 = cur[i + 1];
            next[i] = Gdiplus::PointF((p0.X + 2.0f * p1.X + p2.X) * 0.25f, (p0.Y + 2.0f * p1.Y + p2.Y) * 0.25f);
        }
        next.back() = cur.back();
        cur.swap(next);
    }
    return cur;
}

} // namespace neon3d
} // namespace mousefx
