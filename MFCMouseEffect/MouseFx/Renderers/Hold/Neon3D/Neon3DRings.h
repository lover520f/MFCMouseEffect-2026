#pragma once

#include "Neon3DMath.h"

#include <gdiplus.h>
#include <algorithm>

namespace mousefx {
namespace neon3d {

inline void DrawGlassRing(Gdiplus::Graphics& g, float cx, float cy, float R, float thick,
                          const Gdiplus::Color& cyan, const Gdiplus::Color& purple, float appear01) {
    using namespace mousefx::render_utils;
    const float a = Clamp01(appear01);

    const float x = cx - R;
    const float y = cy - R;
    const float d = R * 2.0f;

    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    // Base shell (thick).
    {
        Gdiplus::Pen pen(MulAlpha(cyan, 0.16f * a), thick);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawEllipse(&pen, x, y, d, d);
    }

    // Soft inner shadow to hint glass depth.
    {
        const float r2 = R - thick * 0.30f;
        const float x2 = cx - r2;
        const float y2 = cy - r2;
        const float d2 = r2 * 2.0f;
        Gdiplus::Pen pen(Gdiplus::Color(ClampByte((int)std::round(255.0f * 0.18f * a)), 0, 0, 0), thick * 0.65f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawEllipse(&pen, x2, y2, d2, d2);
    }

    // Fog glow (approx. "shadowBlur" + "lighter").
    {
        Gdiplus::Pen pen(MulAlpha(cyan, 0.06f * a), thick + 14.0f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawEllipse(&pen, x, y, d, d);
    }
    {
        Gdiplus::Pen pen(MulAlpha(cyan, 0.10f * a), thick + 8.0f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawEllipse(&pen, x, y, d, d);
    }
    {
        Gdiplus::Pen pen(MulAlpha(cyan, 0.045f * a), thick + 20.0f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawEllipse(&pen, x, y, d, d);
    }

    // Rim highlights.
    {
        const float r = R - thick * 0.45f;
        const float x1 = cx - r;
        const float y1 = cy - r;
        const float d1 = r * 2.0f;
        Gdiplus::Pen pen(MulAlpha(Gdiplus::Color(255, 255, 255, 255), 0.22f * a), 2.0f);
        g.DrawEllipse(&pen, x1, y1, d1, d1);
    }
    {
        const float r = R + thick * 0.45f;
        const float x1 = cx - r;
        const float y1 = cy - r;
        const float d1 = r * 2.0f;
        Gdiplus::Pen pen(MulAlpha(cyan, 0.18f * a), 2.0f);
        g.DrawEllipse(&pen, x1, y1, d1, d1);
    }

    // Very faint purple tint (concept has cyan+purple interplay).
    {
        Gdiplus::Pen pen(MulAlpha(purple, 0.06f * a), thick * 0.55f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawEllipse(&pen, x, y, d, d);
    }
}

inline void DrawStartAnchorPlate(Gdiplus::Graphics& g, float cx, float cy, float R, float startAngRad,
                                 const Gdiplus::Color& cyan, const Gdiplus::Color& purple, float alpha) {
    using namespace mousefx::render_utils;
    const float a = Clamp01(alpha);
    if (a <= 0.001f) return;

    // Concept detail: a subtle HUD "plate" at the fixed start (12 o'clock).
    const float plateSweep = 0.18f;
    const float x = cx - R;
    const float y = cy - R;
    const float d = R * 2.0f;

    {
        Gdiplus::Pen glow(MulAlpha(purple, 0.05f * a), 7.5f);
        glow.SetStartCap(Gdiplus::LineCapSquare);
        glow.SetEndCap(Gdiplus::LineCapSquare);
        g.DrawArc(&glow, x, y, d, d, RadToDeg(startAngRad - plateSweep * 0.5f), RadToDeg(plateSweep));
    }
    {
        Gdiplus::Pen core(MulAlpha(cyan, 0.13f * a), 3.8f);
        core.SetStartCap(Gdiplus::LineCapSquare);
        core.SetEndCap(Gdiplus::LineCapSquare);
        g.DrawArc(&core, x, y, d, d, RadToDeg(startAngRad - plateSweep * 0.5f), RadToDeg(plateSweep));
    }
    {
        Gdiplus::Pen line(MulAlpha(Gdiplus::Color(255, 245, 252, 255), 0.18f * a), 1.2f);
        line.SetStartCap(Gdiplus::LineCapSquare);
        line.SetEndCap(Gdiplus::LineCapSquare);
        g.DrawArc(&line, x, y, d, d, RadToDeg(startAngRad - plateSweep * 0.5f), RadToDeg(plateSweep));
    }
}

inline void DrawProgressArcBase(Gdiplus::Graphics& g, float cx, float cy, float R, float progress01,
                                const Gdiplus::Color& cyan, float alpha) {
    using namespace mousefx::render_utils;
    const float p = Clamp01(progress01);
    if (p <= 0.0001f) return;

    const float x = cx - R;
    const float y = cy - R;
    const float d = R * 2.0f;

    Gdiplus::Pen pen(MulAlpha(cyan, 0.10f * alpha), 2.0f);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    g.DrawArc(&pen, x, y, d, d, -90.0f, p * 360.0f);
}

inline void DrawProgressArcBase(Gdiplus::Graphics& g, float cx, float cy, float R, float startAngRad, float progress01,
                                const Gdiplus::Color& cyan, float alpha) {
    using namespace mousefx::render_utils;
    const float p = Clamp01(progress01);
    if (p <= 0.0001f) return;

    const float x = cx - R;
    const float y = cy - R;
    const float d = R * 2.0f;

    Gdiplus::Pen pen(MulAlpha(cyan, 0.10f * alpha), 2.0f);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    g.DrawArc(&pen, x, y, d, d, RadToDeg(startAngRad), p * 360.0f);
}

inline void DrawProgressMainArc(Gdiplus::Graphics& g, float cx, float cy, float R, float startAngRad, float progress01,
                                const Gdiplus::Color& cyan, const Gdiplus::Color& purple, float alpha) {
    using namespace mousefx::render_utils;
    const float p = Clamp01(progress01);
    if (p <= 0.001f) return;

    const float x = cx - R;
    const float y = cy - R;
    const float d = R * 2.0f;

    const float startDeg = RadToDeg(startAngRad);
    const float sweepDeg = p * 360.0f;

    auto arc = [&](float w, const Gdiplus::Color& c, float a01) {
        Gdiplus::Pen pen(MulAlpha(c, a01 * alpha), w);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&pen, x, y, d, d, startDeg, sweepDeg);
    };

    // This is the "readable progress" arc (concept: one end fixed, the other grows to head).
    // Keep it much softer than the woven band + capsule head.
    const float coreFade = 1.0f - Smoothstep(0.25f, 0.55f, p); // visible early, subtle later
    arc(14.0f, purple, 0.025f);
    arc(10.0f, cyan,   0.040f);
    arc(6.5f,  cyan,   0.075f);
    arc(2.6f,  Gdiplus::Color(255, 235, 248, 255), 0.28f * coreFade);
}

inline void DrawCapsuleHead(Gdiplus::Graphics& g, float cx, float cy, float R, float progress01, float headAngRad,
                            const Gdiplus::Color& cyan, const Gdiplus::Color& purple, float alpha) {
    using namespace mousefx::render_utils;
    const float p = Clamp01(progress01);
    if (p <= 0.0001f) return;

    const float sweep = 0.46f; // rad (~26deg)
    const float a0 = headAngRad - sweep;

    const float x = cx - R;
    const float y = cy - R;
    const float d = R * 2.0f;

    const float startDeg = RadToDeg(a0);
    const float sweepDeg = RadToDeg(sweep);

    auto arc = [&](float w, const Gdiplus::Color& c, float a01) {
        Gdiplus::Pen pen(MulAlpha(c, a01 * alpha), w);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&pen, x, y, d, d, startDeg, sweepDeg);
    };

    // Glow layers (approx of canvas shadowBlur).
    arc(18.0f, purple, 0.06f);
    arc(12.0f, purple, 0.16f);
    arc(12.0f, cyan, 0.07f);
    arc(8.0f,  cyan, 0.20f);

    // Core bright stroke.
    arc(3.2f, Gdiplus::Color(255, 245, 250, 255), 0.90f);

    // Head dot.
    const Gdiplus::PointF hp = Polar(cx, cy, R, headAngRad);
    {
        Gdiplus::SolidBrush b(MulAlpha(Gdiplus::Color(255, 245, 250, 255), 0.95f * alpha));
        g.FillEllipse(&b, hp.X - 2.6f, hp.Y - 2.6f, 5.2f, 5.2f);
    }
    {
        Gdiplus::SolidBrush b(MulAlpha(cyan, 0.18f * alpha));
        g.FillEllipse(&b, hp.X - 7.0f, hp.Y - 7.0f, 14.0f, 14.0f);
    }
}

} // namespace neon3d
} // namespace mousefx
