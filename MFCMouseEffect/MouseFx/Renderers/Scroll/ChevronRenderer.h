#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include <cmath>

namespace mousefx {

class ChevronRenderer : public IRippleRenderer {
public:
    void SetParams(const RenderParams& params) override { params_ = params; }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        
        const float tn = Clamp01(t);
        const float eased = 1.0f - (1.0f - tn) * (1.0f - tn) * (1.0f - tn);
        const float intensity = Clamp01(params_.intensity);
        const float alpha = (1.0f - eased) * intensity;
        const float radius = style.startRadius + (style.endRadius - style.startRadius) * eased;
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;

        const float dir = params_.directionRad;
        const float dx = (float)cos(dir);
        const float dy = (float)sin(dir);
        const float px = -dy;
        const float py = dx;

        const Gdiplus::Color base = ToGdiPlus(style.stroke);
        const BYTE aBase = ClampByte((int)(base.GetA() * alpha));

        // Subtle forward drift makes it feel less static.
        const float drift = radius * 0.10f * (1.0f - eased);

        for (int i = 0; i < 3; ++i) {
            const float offset = i * (radius * 0.26f) + drift;
            const float length = radius * 1.12f;
            const float halfWidth = style.strokeWidth * (3.1f - i * 0.65f);

            const float tipX = cx + dx * (length * 0.5f - offset);
            const float tipY = cy + dy * (length * 0.5f - offset);
            const float tailX = cx - dx * (length * 0.5f + offset);
            const float tailY = cy - dy * (length * 0.5f + offset);

            const float lx = tailX + px * halfWidth;
            const float ly = tailY + py * halfWidth;
            const float rx = tailX - px * halfWidth;
            const float ry = tailY - py * halfWidth;

            const float fade = 1.0f - i * 0.20f;
            const Gdiplus::Color glow = ToGdiPlus(style.glow);
            const BYTE a = ClampByte((int)(aBase * fade));
            const BYTE glowA = ClampByte((int)(glow.GetA() * alpha * fade));

            // Two-layer glow: softer outer + tighter inner.
            {
                Gdiplus::Pen glowPen(Gdiplus::Color(ClampByte((int)(glowA * 0.55f)), glow.GetR(), glow.GetG(), glow.GetB()),
                    style.strokeWidth + 9.0f);
                glowPen.SetStartCap(Gdiplus::LineCapRound);
                glowPen.SetEndCap(Gdiplus::LineCapRound);
                glowPen.SetLineJoin(Gdiplus::LineJoinRound);
                g.DrawLine(&glowPen, tipX, tipY, lx, ly);
                g.DrawLine(&glowPen, tipX, tipY, rx, ry);
            }
            {
                Gdiplus::Pen glowPen(Gdiplus::Color(glowA, glow.GetR(), glow.GetG(), glow.GetB()),
                    style.strokeWidth + 5.5f);
                glowPen.SetStartCap(Gdiplus::LineCapRound);
                glowPen.SetEndCap(Gdiplus::LineCapRound);
                glowPen.SetLineJoin(Gdiplus::LineJoinRound);
                g.DrawLine(&glowPen, tipX, tipY, lx, ly);
                g.DrawLine(&glowPen, tipX, tipY, rx, ry);
            }

            Gdiplus::Pen pen(Gdiplus::Color(a, base.GetR(), base.GetG(), base.GetB()),
                style.strokeWidth + 1.0f);
            pen.SetStartCap(Gdiplus::LineCapRound);
            pen.SetEndCap(Gdiplus::LineCapRound);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawLine(&pen, tipX, tipY, lx, ly);
            g.DrawLine(&pen, tipX, tipY, rx, ry);
        }
    }

private:
    RenderParams params_{};
};

REGISTER_RENDERER("arrow", ChevronRenderer)

} // namespace mousefx
