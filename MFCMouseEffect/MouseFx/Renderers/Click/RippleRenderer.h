#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include <cmath>

namespace mousefx {

class RippleRenderer : public IRippleRenderer {
public:
    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;

        const float clampedT = Clamp01(t);
        const float eased = 1.0f - (1.0f - clampedT) * (1.0f - clampedT) * (1.0f - clampedT);
        const float fadeAlpha = std::max(0.0f, 1.0f - clampedT);
        const float radius = style.startRadius + (style.endRadius - style.startRadius) * eased;
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;
        const float ringWidth = std::max(
            style.strokeWidth * 1.34f,
            std::min(radius * 0.18f, std::max(style.strokeWidth + 1.25f, 2.6f)));
        const float innerRadius = std::max(0.0f, radius - ringWidth);
        const float secondaryRadius = std::max(style.startRadius, radius * 0.72f);
        const float secondaryAlpha = fadeAlpha * 0.42f;

        // Fill the ring body rather than the full disc so the center stays cleaner.
        Gdiplus::GraphicsPath ringPath(Gdiplus::FillModeAlternate);
        ringPath.AddEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        if (innerRadius > 0.5f) {
            ringPath.AddEllipse(
                cx - innerRadius,
                cy - innerRadius,
                innerRadius * 2.0f,
                innerRadius * 2.0f);
        }
        Gdiplus::Color fill = ToGdiPlus(style.fill);
        fill = Gdiplus::Color(
            ClampByte(static_cast<int>(fill.GetA() * fadeAlpha * 0.76f)),
            fill.GetR(),
            fill.GetG(),
            fill.GetB());
        Gdiplus::SolidBrush fillBrush(fill);
        g.FillPath(&fillBrush, &ringPath);

        Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        stroke = Gdiplus::Color(
            ClampByte(static_cast<int>(stroke.GetA() * fadeAlpha)),
            stroke.GetR(),
            stroke.GetG(),
            stroke.GetB());

        // Outer glow halo.
        const Gdiplus::Color glow = ToGdiPlus(style.glow);
        for (int i = 0; i < 3; ++i) {
            const float w = ringWidth + 5.0f + i * 2.8f;
            const BYTE a = ClampByte(static_cast<int>(glow.GetA() * fadeAlpha * (0.15f - i * 0.03f)));
            Gdiplus::Pen p(Gdiplus::Color(a, glow.GetR(), glow.GetG(), glow.GetB()), w);
            p.SetLineJoin(Gdiplus::LineJoinRound);
            p.SetStartCap(Gdiplus::LineCapRound);
            p.SetEndCap(Gdiplus::LineCapRound);
            g.DrawEllipse(&p, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        }

        // Secondary inner ring gives a crisper ClickShow-like contour.
        if (secondaryRadius > innerRadius + 1.0f) {
            Gdiplus::Pen secondaryPen(
                Gdiplus::Color(
                    ClampByte(static_cast<int>(stroke.GetA() * secondaryAlpha)),
                    stroke.GetR(),
                    stroke.GetG(),
                    stroke.GetB()),
                std::max(0.9f, style.strokeWidth * 0.78f));
            secondaryPen.SetLineJoin(Gdiplus::LineJoinRound);
            secondaryPen.SetStartCap(Gdiplus::LineCapRound);
            secondaryPen.SetEndCap(Gdiplus::LineCapRound);
            g.DrawEllipse(
                &secondaryPen,
                cx - secondaryRadius,
                cy - secondaryRadius,
                secondaryRadius * 2.0f,
                secondaryRadius * 2.0f);
        }

        Gdiplus::Pen pen(stroke, std::max(style.strokeWidth * 0.94f, ringWidth * 0.18f));
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawEllipse(&pen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }
};

REGISTER_RENDERER("ripple", RippleRenderer)

} // namespace mousefx
