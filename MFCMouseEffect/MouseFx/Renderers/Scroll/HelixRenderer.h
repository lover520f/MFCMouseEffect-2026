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
        const float easeOut = 1.0f - (1.0f - tn) * (1.0f - tn);
        const float lifeAlpha = (1.0f - easeOut);
        const float alphaBase = lifeAlpha * intensity;

        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;

        const float dir = params_.directionRad + 3.1415926f;
        const float dx = (float)cos(dir);
        const float dy = (float)sin(dir);
        const float px = -dy; 
        const float py = dx;

        const float seconds = (float)elapsedMs / 1000.0f;
        const float phase = seconds * 2.2f * 3.1415926f;

        const Gdiplus::Color core = ToGdiPlus(style.stroke);
        const Gdiplus::Color glow = ToGdiPlus(style.glow);

        const int segments = 48;
        const float length = (style.endRadius - style.startRadius) * 2.35f;
        const float widthAmp = style.strokeWidth * 3.2f + 10.0f;

        std::vector<Segment> renderQueue;
        renderQueue.reserve(segments * 2);

        struct Point3D { float x, y, z, w, a; };
        
        auto getPoint = [&](int i, float offsetRad) -> Point3D {
            const float progress = (float)i / (float)segments; // 0..1

            // Slide the helix forward a bit over its lifetime (subtle "flow").
            const float dist = (progress - 0.5f) * length + (1.0f - lifeAlpha) * (length * 0.18f);

            const float angle = progress * 3.0f * 3.1415926f - phase + offsetRad;

            const float lateral = sin(angle) * widthAmp;
            const float z = cos(angle); // -1..1

            const float zFactor = 0.85f + z * 0.25f;

            const float sx = cx + dx * dist + px * lateral;
            const float sy = cy + dy * dist + py * lateral;

            // Taper: thin tail, thicker head.
            const float taper = 0.25f + 0.75f * progress;
            const float alphaFactor = taper * (0.55f + 0.45f * (z * 0.5f + 0.5f));

            return { sx, sy, z, zFactor * taper, alphaFactor };
        };

        for (int i = 0; i < segments - 1; ++i) {
            Point3D p1 = getPoint(i, 0.0f);
            Point3D p2 = getPoint(i + 1, 0.0f);
            
            float avgZ =(p1.z + p2.z) * 0.5f;
            float avgW = (p1.w + p2.w) * 0.5f * style.strokeWidth;
            float avgA = (p1.a + p2.a) * 0.5f * alphaBase;
            
            renderQueue.push_back({ 
                avgZ, p1.x, p1.y, p2.x, p2.y, avgW, 
                Gdiplus::Color(ClampByte((int)(avgA * 255)), core.GetR(), core.GetG(), core.GetB()) 
            });

            Point3D q1 = getPoint(i, 3.14159f);
            Point3D q2 = getPoint(i + 1, 3.14159f);

            float avgZ2 =(q1.z + q2.z) * 0.5f;
            float avgW2 = (q1.w + q2.w) * 0.5f * style.strokeWidth;
            float avgA2 = (q1.a + q2.a) * 0.5f * alphaBase;

            renderQueue.push_back({ 
                avgZ2, q1.x, q1.y, q2.x, q2.y, avgW2, 
                Gdiplus::Color(ClampByte((int)(avgA2 * 255)), glow.GetR(), glow.GetG(), glow.GetB()) 
            });
        }

        std::sort(renderQueue.begin(), renderQueue.end(), [](const Segment& a, const Segment& b) {
            return a.z < b.z;
        });

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        for (const auto& s : renderQueue) {
            if (s.color.GetA() == 0) continue;

            // Soft aura
            {
                const BYTE auraA = ClampByte((int)(s.color.GetA() * 0.35f));
                Gdiplus::Pen auraPen(Gdiplus::Color(auraA, glow.GetR(), glow.GetG(), glow.GetB()),
                    (s.width + 6.0f) < 1.0f ? 1.0f : (s.width + 6.0f));
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

        // Head highlight (small, non-blinding).
        for (float offset : {0.0f, 3.1415926f}) {
            const Point3D head = getPoint(segments - 1, offset);
            const int headA = ClampByte((int)(alphaBase * 255));

            const float r = style.strokeWidth * 1.35f;
            Gdiplus::SolidBrush coreBrush(Gdiplus::Color(ClampByte((int)(headA * 0.85f)), 255, 255, 255));
            g.FillEllipse(&coreBrush, head.x - r, head.y - r, r * 2, r * 2);

            const float r2 = r * 2.2f;
            Gdiplus::SolidBrush glowBrush(Gdiplus::Color(ClampByte((int)(headA * 0.22f)), glow.GetR(), glow.GetG(), glow.GetB()));
            g.FillEllipse(&glowBrush, head.x - r2, head.y - r2, r2 * 2, r2 * 2);
        }
    }

private:
    RenderParams params_{};
};

REGISTER_RENDERER("helix", HelixRenderer)

} // namespace mousefx
