#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace mousefx {

class HelixRenderer : public IRippleRenderer {
public:
    void SetParams(const RenderParams& params) override { params_ = params; }

    static float Smoothstep(float a, float b, float x) {
        if (b <= a) return x >= b ? 1.0f : 0.0f;
        float t = (x - a) / (b - a);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return t * t * (3.0f - 2.0f * t);
    }

    static Gdiplus::Color MixColor(const Gdiplus::Color& a, const Gdiplus::Color& b, float t, BYTE alpha) {
        using namespace render_utils;
        const float k = Clamp01(t);
        const int r = (int)std::lround((float)a.GetR() * (1.0f - k) + (float)b.GetR() * k);
        const int g = (int)std::lround((float)a.GetG() * (1.0f - k) + (float)b.GetG() * k);
        const int bb = (int)std::lround((float)a.GetB() * (1.0f - k) + (float)b.GetB() * k);
        return Gdiplus::Color(alpha, ClampByte(r), ClampByte(g), ClampByte(bb));
    }

    struct Segment {
        float z;
        float x1, y1, x2, y2;
        float width;
        Gdiplus::Color color;
    };

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;

        const float intensity = Clamp01(params_.intensity);
        if (intensity <= 0.01f) return;

        const float tn = Clamp01(t);
        const float fadeOut = 1.0f - Smoothstep(0.72f, 1.0f, tn);
        const float alphaBase = fadeOut * intensity;
        if (alphaBase <= 0.001f) return;

        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;

        const float dir = params_.directionRad + 3.1415926f;
        const float dx = (float)cos(dir);
        const float dy = (float)sin(dir);
        const float px = -dy; 
        const float py = dx;

        const float seconds = (float)elapsedMs / 1000.0f;
        const float phase = seconds * 1.55f * 3.1415926f;

        const Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        const Gdiplus::Color glow = ToGdiPlus(style.glow);
        const Gdiplus::Color white(255, 255, 255, 255);

        const int segments = 64;
        const float length = (style.endRadius - style.startRadius) * 2.05f;
        const float radius = style.strokeWidth * 2.2f + 6.2f;
        const float camera = radius * 5.2f;
        const float depthAmp = radius * 0.95f;

        std::vector<Segment> renderQueue;
        renderQueue.reserve(segments * 3);

        struct Point3D { float x, y, z, w, a, u; };
        
        auto getPoint = [&](int i, float offsetRad) -> Point3D {
            const float u = (float)i / (float)segments; // 0..1
            const float dist = (u - 0.5f) * length + (1.0f - fadeOut) * (length * 0.14f);
            const float turns = 1.72f;
            const float angle = u * turns * 6.2831853f - phase + offsetRad;

            const float localY = (float)sin(angle) * radius;
            const float localZ = (float)cos(angle) * depthAmp;
            const float persp = camera / (camera + localZ + depthAmp);

            const float sx = cx + dx * dist + px * (localY * persp);
            const float sy = cy + dy * dist + py * (localY * persp);

            const float taper = 0.30f + 0.70f * u;
            const float depthLight = 0.52f + 0.48f * ((localZ / depthAmp) * 0.5f + 0.5f);
            const float headBoost = 0.62f + 0.38f * Smoothstep(0.55f, 1.0f, u);
            const float alphaFactor = taper * depthLight * headBoost;
            const float widthFactor = (0.62f + 0.38f * persp) * taper;
            return { sx, sy, localZ / depthAmp, widthFactor, alphaFactor, u };
        };

        std::vector<Point3D> strandA;
        std::vector<Point3D> strandB;
        strandA.reserve((size_t)segments);
        strandB.reserve((size_t)segments);
        for (int i = 0; i < segments; ++i) {
            strandA.push_back(getPoint(i, 0.0f));
            strandB.push_back(getPoint(i, 3.1415926f));
        }

        for (int i = 0; i < segments - 1; ++i) {
            Point3D p1 = strandA[(size_t)i];
            Point3D p2 = strandA[(size_t)i + 1];
            
            float avgZ = (p1.z + p2.z) * 0.5f;
            float avgW = ((p1.w + p2.w) * 0.5f) * (style.strokeWidth + 0.9f);
            float avgA = (p1.a + p2.a) * 0.5f * alphaBase;
            float avgU = (p1.u + p2.u) * 0.5f;
            const float strandMix = 0.25f + 0.45f * avgU;
            Gdiplus::Color c1 = MixColor(stroke, white, strandMix, ClampByte((int)(avgA * 255.0f)));
             
            renderQueue.push_back({ 
                avgZ, p1.x, p1.y, p2.x, p2.y, avgW, 
                c1
            });

            Point3D q1 = strandB[(size_t)i];
            Point3D q2 = strandB[(size_t)i + 1];

            float avgZ2 = (q1.z + q2.z) * 0.5f;
            float avgW2 = ((q1.w + q2.w) * 0.5f) * (style.strokeWidth + 0.9f);
            float avgA2 = (q1.a + q2.a) * 0.5f * alphaBase;
            float avgU2 = (q1.u + q2.u) * 0.5f;
            const float strandMix2 = 0.20f + 0.32f * avgU2;
            Gdiplus::Color c2 = MixColor(glow, white, strandMix2, ClampByte((int)(avgA2 * 255.0f)));

            renderQueue.push_back({ 
                avgZ2, q1.x, q1.y, q2.x, q2.y, avgW2, 
                c2
            });

            // Sparse rungs improve "double-helix" legibility without adding heavy clutter.
            if (i % 8 == 0) {
                const float rz = (avgZ + avgZ2) * 0.5f - 0.015f;
                const float rw = std::max(1.0f, (style.strokeWidth + 0.5f) * 0.34f);
                const float ra = Clamp01((avgA + avgA2) * 0.34f);
                const Gdiplus::Color rc = MixColor(stroke, glow, 0.5f, ClampByte((int)(ra * 255.0f)));
                renderQueue.push_back({
                    rz, p1.x, p1.y, q1.x, q1.y, rw, rc
                });
            }
        }

        std::sort(renderQueue.begin(), renderQueue.end(), [](const Segment& a, const Segment& b) {
            return a.z < b.z;
        });

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        for (const auto& s : renderQueue) {
            if (s.color.GetA() == 0) continue;

            // Soft aura
            {
                const BYTE auraA = ClampByte((int)(s.color.GetA() * 0.30f));
                Gdiplus::Pen auraPen(Gdiplus::Color(auraA, s.color.GetR(), s.color.GetG(), s.color.GetB()),
                    (s.width + 2.8f) < 1.0f ? 1.0f : (s.width + 2.8f));
                auraPen.SetStartCap(Gdiplus::LineCapRound);
                auraPen.SetEndCap(Gdiplus::LineCapRound);
                g.DrawLine(&auraPen, s.x1, s.y1, s.x2, s.y2);
            }

            // Core line
            {
                Gdiplus::Pen pen(s.color, s.width < 1.0f ? 1.0f : s.width);
                pen.SetStartCap(Gdiplus::LineCapRound);
                pen.SetEndCap(Gdiplus::LineCapRound);
                g.DrawLine(&pen, s.x1, s.y1, s.x2, s.y2);
            }
        }

        // Head highlights (small and crisp).
        for (float offset : {0.0f, 3.1415926f}) {
            const Point3D head = getPoint(segments - 1, offset);
            const int headA = ClampByte((int)(alphaBase * 255));

            const float r = std::max(1.3f, style.strokeWidth * 0.95f);
            Gdiplus::SolidBrush coreBrush(Gdiplus::Color(ClampByte((int)(headA * 0.82f)), 255, 255, 255));
            g.FillEllipse(&coreBrush, head.x - r, head.y - r, r * 2, r * 2);

            const float r2 = r * 1.85f;
            Gdiplus::SolidBrush glowBrush(Gdiplus::Color(ClampByte((int)(headA * 0.18f)), stroke.GetR(), stroke.GetG(), stroke.GetB()));
            g.FillEllipse(&glowBrush, head.x - r2, head.y - r2, r2 * 2, r2 * 2);
        }
    }

private:
    RenderParams params_{};
};

REGISTER_RENDERER("helix", HelixRenderer)

} // namespace mousefx
