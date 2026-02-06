#pragma once

#include "Neon3DBezier.h"
#include "Neon3DRings.h"

#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace mousefx {
namespace neon3d {

static const float kTau = 6.2831853f;
static const float kStartAng = -3.1415926f / 2.0f; // 12 o'clock, clockwise

struct Spark {
    float ang = 0.0f;
    float rr = 0.0f;
    float v = 0.0f;
    float life = 0.0f;
};

inline void DrawMicroStreaks(Gdiplus::Graphics& g, float cx, float cy, float r, float appear01, float timeSec, const Gdiplus::Color& cyan) {
    using namespace mousefx::render_utils;
    const float appear = Clamp01(appear01);
    if (appear <= 0.001f) return;

    const float sweepMax = kTau * 0.30f;
    const float sweep = sweepMax * (0.65f + 0.35f * (float)sin(timeSec * 0.9f) * 0.5f) * appear;

    const float x = cx - r;
    const float y = cy - r;
    const float d = r * 2.0f;

    const int n = 75;
    for (int i = 0; i < n; ++i) {
        const float u = (float)i / (float)(n - 1);
        const float a = kStartAng + sweep * u;
        const float len = (0.08f + 0.06f * (float)sin(timeSec * 2.0f + u * 9.0f)) * 0.18f;
        const float alpha = (0.04f + 0.10f * (1.0f - u)) * appear;

        Gdiplus::Pen pen(MulAlpha(cyan, alpha), 1.1f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&pen, x, y, d, d, RadToDeg(a), RadToDeg(len));
    }

    // Scan peak near the start region.
    {
        const float scanA = kStartAng + fnmod(timeSec * 0.7f, 1.0f) * sweepMax;
        Gdiplus::Pen pen(MulAlpha(Gdiplus::Color(255, 220, 250, 255), 0.22f * appear), 2.2f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&pen, x, y, d, d, RadToDeg(scanA - 0.06f), RadToDeg(0.12f));

        Gdiplus::Pen glow(MulAlpha(cyan, 0.10f * appear), 7.0f);
        glow.SetStartCap(Gdiplus::LineCapRound);
        glow.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&glow, x, y, d, d, RadToDeg(scanA - 0.06f), RadToDeg(0.12f));
    }
}

inline void DrawInnerHatch(Gdiplus::Graphics& g, float cx, float cy, float innerR, float progress01, float headAng, float timeSec,
                           const Gdiplus::Color& cyan) {
    using namespace mousefx::render_utils;
    const float progress = Clamp01(progress01);
    if (progress <= 0.02f) return;

    const float hatchFade = Smoothstep(0.08f, 0.25f, progress);
    const float span = Lerp(0.55f, 0.85f, Smoothstep(0.15f, 1.0f, progress));
    const float a0 = headAng - span;
    const float a1 = headAng + 0.10f;

    const int lines = 46;
    for (int i = 0; i < lines; ++i) {
        const float u = (float)i / (float)(lines - 1);
        const float a = Lerp(a0, a1, u);

        const float rr = innerR - 10.0f + 6.0f * (float)sin(timeSec * 2.2f + u * 10.0f);
        const Gdiplus::PointF p = Polar(cx, cy, rr, a);

        const float tilt = a + 3.1415926f / 2.0f + 0.55f;
        const float L = 10.0f + 8.0f * (1.0f - u);
        const Gdiplus::PointF q(p.X + (float)cos(tilt) * L, p.Y + (float)sin(tilt) * L);

        const float alpha = (0.04f + 0.10f * (1.0f - u)) * hatchFade;
        Gdiplus::Pen pen(MulAlpha(cyan, alpha), 1.0f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&pen, p, q);
    }

    // Inner edge reflection arc.
    {
        const float r = innerR - 6.0f;
        const float x = cx - r;
        const float y = cy - r;
        const float d = r * 2.0f;
        Gdiplus::Pen pen(MulAlpha(Gdiplus::Color(255, 255, 255, 255), 0.08f * hatchFade), 1.6f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&pen, x, y, d, d, RadToDeg(a0), RadToDeg(a1 - a0));
    }
}

inline void DrawCrystalSeed(Gdiplus::Graphics& g, float cx, float cy, float size, float timeSec, float appear01,
                            const Gdiplus::Color& cyan, const Gdiplus::Color& purple) {
    using namespace mousefx::render_utils;
    const float appear = Clamp01(appear01);
    if (appear <= 0.001f) return;

    const float pulse = 0.5f + 0.5f * (float)sin(timeSec * 6.0f);
    const float scale = 0.88f + 0.12f * EaseOutBack(appear);

    const float w = size * 1.05f * scale;
    const float h = size * 1.35f * scale;

    const Gdiplus::PointF top(cx, cy - h * 0.55f);
    const Gdiplus::PointF right(cx + w * 0.55f, cy);
    const Gdiplus::PointF bottom(cx, cy + h * 0.55f);
    const Gdiplus::PointF left(cx - w * 0.55f, cy);

    // Outer shell.
    {
        Gdiplus::PointF pts[] = { top, right, bottom, left };
        Gdiplus::GraphicsPath path;
        path.AddClosedCurve(pts, 4, 0.25f);

        Gdiplus::PathGradientBrush pgb(&path);
        const float aCenter = (0.70f + 0.18f * pulse) * appear;
        pgb.SetCenterColor(MulAlpha(cyan, aCenter));

        Gdiplus::Color surround[] = { MulAlpha(purple, (0.22f + 0.08f * pulse) * appear) };
        int n = 1;
        pgb.SetSurroundColors(surround, &n);
        g.FillPath(&pgb, &path);

        // Shell edge glow (multi-pass).
        {
            Gdiplus::Pen glow(MulAlpha(purple, 0.22f * appear), 9.0f);
            glow.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawPath(&glow, &path);
        }
        {
            Gdiplus::Pen pen(MulAlpha(cyan, 0.18f * appear), 3.2f);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawPath(&pen, &path);
        }
    }

    // Inner core.
    {
        const float r = size * 0.46f;
        Gdiplus::GraphicsPath p;
        p.AddEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);
        Gdiplus::PathGradientBrush pgb(&p);
        pgb.SetCenterColor(MulAlpha(purple, 0.55f * appear));
        Gdiplus::Color surround[] = { MulAlpha(purple, 0.0f) };
        int n = 1;
        pgb.SetSurroundColors(surround, &n);
        g.FillPath(&pgb, &p);
    }

    // Vertical refraction highlight.
    {
        Gdiplus::Pen pen(MulAlpha(Gdiplus::Color(255, 255, 255, 255), 0.20f * appear), 1.5f);
        g.DrawLine(&pen, cx - w * 0.10f, cy - h * 0.25f, cx - w * 0.02f, cy + h * 0.25f);
    }
}

inline void UpdateSparks(std::vector<Spark>& sparks, float dtSec) {
    if (dtSec <= 0.0f) return;
    for (auto& s : sparks) {
        s.life -= dtSec;
        s.rr += s.v * dtSec;
    }
    sparks.erase(std::remove_if(sparks.begin(), sparks.end(), [](const Spark& s) { return s.life <= 0.0f; }), sparks.end());
    if (sparks.size() > 120) sparks.erase(sparks.begin(), sparks.begin() + (sparks.size() - 120));
}

inline void DrawBranchTendrils(Gdiplus::Graphics& g, float cx, float cy, float innerR, float progress01, float headAng, float timeSec, float dtSec,
                               float alpha01, float bundleBiasRad,
                               const Gdiplus::Color& cyan, const Gdiplus::Color& purple, const float* seeds, int seedsCount,
                               int branchCount, std::vector<Spark>& sparks, LcgRng& rng) {
    using namespace mousefx::render_utils;
    (void)cyan;
    const float alpha = Clamp01(alpha01);
    const float progress = Clamp01(progress01);
    if (progress <= 0.02f || alpha <= 0.001f) return;

    UpdateSparks(sparks, dtSec);

    const Gdiplus::PointF start(cx, cy);
    const float bundleWob = 0.12f * (float)sin(timeSec * 1.15f + bundleBiasRad * 2.7f);
    const float bundleAng = headAng - 0.22f + bundleBiasRad + bundleWob;
    const Gdiplus::PointF trunkEnd = Polar(cx, cy, innerR - 12.0f, bundleAng);

    // Trunk control points (slight arc).
    const Gdiplus::PointF c1(start.X + (trunkEnd.X - start.X) * 0.35f + 6.0f * (float)sin(timeSec * 2.0f),
                             start.Y + (trunkEnd.Y - start.Y) * 0.35f - 10.0f);
    const Gdiplus::PointF c2(start.X + (trunkEnd.X - start.X) * 0.75f + 10.0f,
                             start.Y + (trunkEnd.Y - start.Y) * 0.75f + 6.0f * (float)cos(timeSec * 1.6f));

    const auto trunkPts = SampleBezier(start, c1, c2, trunkEnd, 22);
    const auto trunkJ = JitterPolyline(trunkPts, timeSec, progress, seeds, seedsCount, 1.0f, 2.5f, 8.0f);
    const int forkIndex = (int)std::floor((float)trunkJ.size() * 0.62f);
    const Gdiplus::PointF forkBase = trunkJ[ClampInt(forkIndex, 0, (int)trunkJ.size() - 1)];

    auto drawPolyline = [&](const std::vector<Gdiplus::PointF>& pts, float wGlow, float aGlow, float wCore, float aCore) {
        if (pts.size() < 2) return;
        {
            Gdiplus::Pen pen(MulAlpha(purple, aGlow * alpha), wGlow);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            pen.SetStartCap(Gdiplus::LineCapRound);
            pen.SetEndCap(Gdiplus::LineCapRound);
            g.DrawLines(&pen, pts.data(), (INT)pts.size());
        }
        {
            Gdiplus::Pen pen(MulAlpha(Gdiplus::Color(255, 235, 250, 255), aCore * alpha), wCore);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            pen.SetStartCap(Gdiplus::LineCapRound);
            pen.SetEndCap(Gdiplus::LineCapRound);
            g.DrawLines(&pen, pts.data(), (INT)pts.size());
        }
    };

    drawPolyline(trunkJ, 7.0f, 0.18f, 2.2f, 0.42f);

    const float spread = Lerp(0.55f, 0.95f, Smoothstep(0.10f, 1.0f, progress));
    const float endA0 = headAng - spread;
    const float endA1 = headAng + 0.04f;

    // Branches.
    for (int i = 0; i < branchCount; ++i) {
        const float u = (branchCount <= 1) ? 0.0f : (float)i / (float)(branchCount - 1);
        const float a = Lerp(endA0, endA1, u);
        const Gdiplus::PointF end = Polar(cx, cy, innerR, a);

        const float dx = end.X - forkBase.X;
        const float dy = end.Y - forkBase.Y;
        float nx = -dy;
        float ny = dx;
        const float nl = std::max(1.0f, (float)std::sqrt(nx * nx + ny * ny));
        nx /= nl; ny /= nl;

        const float branchAmp = (10.0f + 18.0f * u) * (0.6f + 0.6f * progress);
        const float wob = (float)sin(timeSec * 7.5f + (seedsCount > 0 ? seeds[(i + 3) % seedsCount] : 0.0f) + u * 9.0f);

        const Gdiplus::PointF p0 = forkBase;
        const Gdiplus::PointF p3 = end;
        const Gdiplus::PointF p1(p0.X + dx * 0.25f + nx * 2.0f, p0.Y + dy * 0.25f + ny * 2.0f);
        const Gdiplus::PointF p2(p0.X + dx * 0.70f + nx * branchAmp * wob, p0.Y + dy * 0.70f + ny * branchAmp * wob);

        const auto raw = SampleBezier(p0, p1, p2, p3, 18);
        std::vector<Gdiplus::PointF> j;
        j.reserve(raw.size());
        for (int k = 0; k < (int)raw.size(); ++k) {
            const float uu = (raw.size() <= 1) ? 0.0f : (float)k / (float)(raw.size() - 1);
            const float aamp = (0.6f + 2.2f * uu) * (0.6f + 0.6f * progress);
            const float ww = (float)sin(timeSec * 9.5f + uu * 12.0f + (seedsCount > 0 ? seeds[(k + i) % seedsCount] : 0.0f));
            const Gdiplus::PointF nn = PolyNormal(raw, k);
            j.push_back(Gdiplus::PointF(raw[k].X + nn.X * ww * aamp, raw[k].Y + nn.Y * ww * aamp));
        }

        drawPolyline(j, 5.5f, 0.18f, 1.8f, 0.42f);

        // Endpoint dot (glow + aura).
        const float dotR = 2.2f + 2.4f * (0.35f + 0.65f * progress);
        {
            Gdiplus::SolidBrush b(MulAlpha(Gdiplus::Color(255, 245, 250, 255), 0.92f * alpha));
            g.FillEllipse(&b, end.X - dotR, end.Y - dotR, dotR * 2.0f, dotR * 2.0f);
        }
        {
            Gdiplus::SolidBrush b(MulAlpha(purple, 0.18f * alpha));
            g.FillEllipse(&b, end.X - dotR * 3.2f, end.Y - dotR * 3.2f, dotR * 6.4f, dotR * 6.4f);
        }

        // Spawn sparks (small, like concept's right-side dots).
        const float spawnChancePerSec = 0.6f * (0.10f + 0.90f * progress);
        if (rng.Next01() < spawnChancePerSec * dtSec) {
            Spark s;
            s.ang = a + (rng.Next01() - 0.5f) * 0.35f;
            s.rr = innerR + 2.0f;
            s.v = 50.0f + 140.0f * rng.Next01();
            s.life = 0.45f + 0.40f * rng.Next01();
            sparks.push_back(s);
        }
    }

    // Draw sparks.
    for (const auto& s : sparks) {
        const float a = Clamp01(s.life / 0.85f);
        const float rr = 1.2f + 2.0f * a;
        const Gdiplus::PointF p = Polar(cx, cy, s.rr, s.ang);
        Gdiplus::SolidBrush b(MulAlpha(Gdiplus::Color(255, 210, 250, 255), 0.55f * a * alpha));
        g.FillEllipse(&b, p.X - rr, p.Y - rr, rr * 2.0f, rr * 2.0f);
    }
}

inline void DrawWovenBand(Gdiplus::Graphics& g, float cx, float cy, float R, float progress01, float headAng, float timeSec,
                          const Gdiplus::Color& cyan, const Gdiplus::Color& purple, const Gdiplus::Color& mint) {
    using namespace mousefx::render_utils;
    const float progress = Clamp01(progress01);
    if (progress <= 0.0f) return;

    // Concept tweak: keep the tail anchored at 12 o'clock (kStartAng) so the weave does not look
    // like a fixed-length segment sliding around the ring. Brightness still concentrates near head.
    const float energyFade = Smoothstep(0.05f, 0.20f, progress);
    const float u0 = kStartAng;
    const float u1 = headAng;
    const float sweep = std::max(0.0f, u1 - u0);
    if (sweep <= 0.01f) return;

    struct Strand { float phase; int m; Gdiplus::Color col; float w; };
    const Strand strands[] = {
        { 0.0f, 2, cyan, 2.6f },
        { 1.7f, 2, purple, 2.3f },
        { 3.2f, 3, mint, 2.0f },
    };

    const int steps = (int)std::floor(160.0f + 420.0f * Clamp01(sweep / kTau));

    struct Seg {
        float z;
        Gdiplus::PointF a;
        Gdiplus::PointF b;
        Gdiplus::Color col;
        float w;
        float a01;
    };

    std::vector<Seg> segs;
    segs.reserve((size_t)_countof(strands) * (size_t)steps);

    // Keep the weave mostly inside the glass shell thickness (GDI+ has no additive blend).
    const float minor = R * 0.10f;
    for (const auto& st : strands) {
        for (int i = 0; i < steps; ++i) {
            const float t0 = (float)i / (float)steps;
            const float t1 = (float)(i + 1) / (float)steps;
            const float uA = Lerp(u0, u1, t0);
            const float uB = Lerp(u0, u1, t1);

            auto P = [&](float u) -> Vec3 {
                const float v = (float)st.m * u + st.phase + timeSec * 0.95f;
                const float xx = (R + minor * (float)cos(v)) * (float)cos(u);
                const float yy = (R + minor * (float)cos(v)) * (float)sin(u);
                const float zz = (minor * (float)sin(v)) * 1.25f;
                return Vec3{ xx, yy, zz };
            };

            const Vec3 A = P(uA);
            const Vec3 B = P(uB);
            const float z = (A.z + B.z) * 0.5f;

            const float headBoost = 0.55f + 0.45f * t0;
            const float a01 = (0.14f + 0.62f * headBoost) * (0.35f + 0.65f * energyFade);

            const float scale = 1.0f + (z / (R * 0.20f)) * 0.07f;
            const Gdiplus::PointF a(cx + A.x * scale, cy + A.y * scale);
            const Gdiplus::PointF b(cx + B.x * scale, cy + B.y * scale);

            segs.push_back(Seg{ z, a, b, st.col, st.w, a01 });
        }
    }

    std::sort(segs.begin(), segs.end(), [](const Seg& a, const Seg& b) { return a.z < b.z; });

    for (const auto& s : segs) {
        // Glow.
        {
            Gdiplus::Pen pen(MulAlpha(s.col, 0.10f * s.a01), s.w + 7.0f);
            pen.SetStartCap(Gdiplus::LineCapRound);
            pen.SetEndCap(Gdiplus::LineCapRound);
            g.DrawLine(&pen, s.a, s.b);
        }
        // Core.
        {
            Gdiplus::Pen pen(MulAlpha(s.col, s.a01), s.w);
            pen.SetStartCap(Gdiplus::LineCapRound);
            pen.SetEndCap(Gdiplus::LineCapRound);
            g.DrawLine(&pen, s.a, s.b);
        }
    }
}

inline void DrawPercentLabel(Gdiplus::Graphics& g, float cx, float cy, float R, float progress01, float headAng,
                             float alpha01, float outwardPx,
                             const Gdiplus::Color& cyan, const Gdiplus::Color& purple) {
    using namespace mousefx::render_utils;
    (void)cyan;
    const float alpha = Clamp01(alpha01);
    const float progress = Clamp01(progress01);
    const int pct = (int)std::round(progress * 100.0f);

    // Clamp around top.
    // Note: the overlay window is ~220px. Use a tighter clamp to avoid clipping.
    const float topMin = DegToRad(-120.0f);
    const float topMax = DegToRad(-60.0f);
    const float a = std::max(topMin, std::min(topMax, headAng - 0.35f));

    const float out = std::max(6.0f, outwardPx);
    const Gdiplus::PointF pos = Polar(cx, cy, R + out, a);
    const Gdiplus::PointF onRing = Polar(cx, cy, R + std::min(8.0f, out * 0.60f), a);

    // Guide line.
    {
        Gdiplus::Pen pen(MulAlpha(purple, 0.18f * alpha), 1.2f);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&pen, onRing, pos);
    }

    wchar_t buf[16]{};
    _snwprintf_s(buf, _countof(buf), _TRUNCATE, L"%d%%", std::max(0, std::min(100, pct)));

    Gdiplus::FontFamily ff(L"Segoe UI");
    Gdiplus::Font font(&ff, 16.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment(Gdiplus::StringAlignmentCenter);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    const Gdiplus::RectF rc(pos.X - 44.0f, pos.Y - 16.0f, 88.0f, 32.0f);

    // Glow text (cheap shadow blur approximation).
    {
        Gdiplus::SolidBrush glow(MulAlpha(purple, 0.55f * alpha));
        const Gdiplus::RectF r1(rc.X + 1.2f, rc.Y + 1.2f, rc.Width, rc.Height);
        const Gdiplus::RectF r2(rc.X - 1.2f, rc.Y + 0.8f, rc.Width, rc.Height);
        g.DrawString(buf, -1, &font, r1, &fmt, &glow);
        g.DrawString(buf, -1, &font, r2, &fmt, &glow);
    }

    // Core text.
    {
        Gdiplus::SolidBrush text(MulAlpha(Gdiplus::Color(255, 230, 245, 255), 0.94f * alpha));
        g.DrawString(buf, -1, &font, rc, &fmt, &text);
    }
}

} // namespace neon3d
} // namespace mousefx
