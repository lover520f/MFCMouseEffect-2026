#pragma once

#include "Neon3DBezier.h"
#include "Neon3DRings.h"
#include "MouseFx/Compute/EffectComputeExecutor.h"

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

inline void DrawInnerScanner(Gdiplus::Graphics& g, float cx, float cy, float r, float timeSec, float alpha01,
                             const Gdiplus::Color& cyan, const Gdiplus::Color& purple) {
    using namespace mousefx::render_utils;
    const float a = Clamp01(alpha01);
    if (a <= 0.001f) return;

    // Flow semantics:
    // 1) streamlines move clockwise along the inner wall
    // 2) 12 o'clock is a fixed outlet with tangential extension.
    const float speed = 0.24f; // rotations per second
    const float travel = fnmod(timeSec * speed, 1.0f);
    const float rr = r - 1.8f;
    const float x = cx - rr;
    const float y = cy - rr;
    const float d = rr * 2.0f;

    // Faint base loop keeps the scanner readable even under dense top layers.
    {
        Gdiplus::Pen base(MulAlpha(cyan, 0.045f * a), 2.2f);
        base.SetStartCap(Gdiplus::LineCapRound);
        base.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&base, x, y, d, d, RadToDeg(kStartAng), 360.0f);
    }
    {
        Gdiplus::Pen base2(MulAlpha(purple, 0.022f * a), 4.5f);
        base2.SetStartCap(Gdiplus::LineCapRound);
        base2.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&base2, x, y, d, d, RadToDeg(kStartAng), 360.0f);
    }

    const int streakCount = 34;
    for (int i = 0; i < streakCount; ++i) {
        const float slot = (float)i / (float)streakCount;
        const float phase = fnmod(slot + travel, 1.0f); // clockwise phase [0..1)
        const float seg = 0.022f + 0.030f * (1.0f - slot);
        const float head = kStartAng + phase * kTau;
        const float start = head - seg;

        // "Flow to outlet": as phase approaches 1.0 (back to 12 o'clock), brightness rises.
        const float towardOutlet = Smoothstep(0.52f, 1.0f, phase);
        const float justAfterOutlet = 1.0f - Smoothstep(0.0f, 0.14f, phase);
        const float energy = (0.08f + 0.92f * towardOutlet) * (1.0f - justAfterOutlet * 0.45f);

        const float glowA = (0.030f + 0.090f * energy) * a;
        const float coreA = (0.055f + 0.190f * energy) * a;

        Gdiplus::Pen glow(MulAlpha(purple, glowA), 4.4f);
        glow.SetStartCap(Gdiplus::LineCapRound);
        glow.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&glow, x, y, d, d, RadToDeg(start), RadToDeg(seg));

        Gdiplus::Pen core(MulAlpha(cyan, coreA), 1.75f);
        core.SetStartCap(Gdiplus::LineCapRound);
        core.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&core, x, y, d, d, RadToDeg(start), RadToDeg(seg));
    }

    // Outlet collar at 12 o'clock, makes the fixed sink point obvious.
    {
        Gdiplus::Pen glow(MulAlpha(purple, 0.11f * a), 8.0f);
        glow.SetStartCap(Gdiplus::LineCapRound);
        glow.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&glow, x, y, d, d, RadToDeg(kStartAng - 0.10f), RadToDeg(0.18f));
    }
    {
        Gdiplus::Pen core(MulAlpha(cyan, 0.20f * a), 3.0f);
        core.SetStartCap(Gdiplus::LineCapRound);
        core.SetEndCap(Gdiplus::LineCapRound);
        g.DrawArc(&core, x, y, d, d, RadToDeg(kStartAng - 0.10f), RadToDeg(0.18f));
    }

    // Fixed 12 o'clock outlet extension (tangent direction = clockwise to the right).
    const Gdiplus::PointF outlet = Polar(cx, cy, rr + 1.0f, kStartAng);
    const Gdiplus::PointF tangent((float)cos(kStartAng + 3.1415926f * 0.5f), (float)sin(kStartAng + 3.1415926f * 0.5f));
    const float outLen = 16.0f + 10.0f * (0.5f + 0.5f * (float)sin(timeSec * 4.0f));
    const Gdiplus::PointF outMid(outlet.X + tangent.X * (outLen * 0.45f), outlet.Y + tangent.Y * (outLen * 0.45f));
    const Gdiplus::PointF outFar(outlet.X + tangent.X * outLen, outlet.Y + tangent.Y * outLen);

    {
        Gdiplus::Pen glow(MulAlpha(purple, 0.17f * a), 10.0f);
        glow.SetStartCap(Gdiplus::LineCapRound);
        glow.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&glow, outlet, outFar);
    }
    {
        Gdiplus::Pen core(MulAlpha(cyan, 0.30f * a), 4.2f);
        core.SetStartCap(Gdiplus::LineCapRound);
        core.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&core, outlet, outFar);
    }
    {
        Gdiplus::Pen line(MulAlpha(Gdiplus::Color(255, 245, 252, 255), 0.52f * a), 1.6f);
        line.SetStartCap(Gdiplus::LineCapRound);
        line.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&line, outMid, outFar);
    }

    {
        const float dotR = 1.9f + 0.8f * (0.5f + 0.5f * (float)sin(timeSec * 5.8f));
        Gdiplus::SolidBrush b(MulAlpha(Gdiplus::Color(255, 245, 252, 255), 0.82f * a));
        g.FillEllipse(&b, outlet.X - dotR, outlet.Y - dotR, dotR * 2.0f, dotR * 2.0f);
    }
}

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

    const float bundleWob = 0.12f * (float)sin(timeSec * 1.15f + bundleBiasRad * 2.7f);
    const float bundleAng = headAng - 0.22f + bundleBiasRad + bundleWob;
    // Start from the crystal edge instead of the exact center (more like the concept "nozzle").
    const Gdiplus::PointF start = Polar(cx, cy, 9.0f, bundleAng);
    const Gdiplus::PointF trunkEnd = Polar(cx, cy, innerR - 12.0f, bundleAng);

    // Trunk control points (slight arc).
    const Gdiplus::PointF c1(start.X + (trunkEnd.X - start.X) * 0.35f + 6.0f * (float)sin(timeSec * 2.0f),
                             start.Y + (trunkEnd.Y - start.Y) * 0.35f - 10.0f);
    const Gdiplus::PointF c2(start.X + (trunkEnd.X - start.X) * 0.75f + 10.0f,
                             start.Y + (trunkEnd.Y - start.Y) * 0.75f + 6.0f * (float)cos(timeSec * 1.6f));

    const auto trunkPts = SampleBezier(start, c1, c2, trunkEnd, 22);
    const auto trunkJRaw = JitterPolyline(trunkPts, timeSec, progress, seeds, seedsCount, 0.45f, 1.35f, 6.0f);

    // Keep the trunk smoother near the crystal and only add jitter toward the ring.
    std::vector<Gdiplus::PointF> trunkJ;
    trunkJ.reserve(trunkPts.size());
    for (size_t i = 0; i < trunkPts.size(); ++i) {
        const float u = (trunkPts.size() <= 1) ? 0.0f : (float)i / (float)(trunkPts.size() - 1);
        const float gate = Smoothstep(0.58f, 1.0f, u);
        trunkJ.push_back(Lerp(trunkPts[i], trunkJRaw[i], gate));
    }
    trunkJ = SmoothPolyline(trunkJ, 2);

    const int forkIndex = (int)std::floor((float)trunkJ.size() * 0.70f);
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

    drawPolyline(trunkJ, 6.0f, 0.14f, 2.0f, 0.34f);

    const float spread = Lerp(0.55f, 0.95f, Smoothstep(0.10f, 1.0f, progress));
    const float endA0 = headAng - spread;
    const float endA1 = headAng + 0.04f;

    struct BranchGeometry {
        float angle = 0.0f;
        Gdiplus::PointF end{};
        std::vector<Gdiplus::PointF> points{};
    };

    auto buildBranch = [&](int i) -> BranchGeometry {
        BranchGeometry out{};

        const float u = (branchCount <= 1) ? 0.0f : (float)i / (float)(branchCount - 1);
        const float a = Lerp(endA0, endA1, u);
        const Gdiplus::PointF end = Polar(cx, cy, innerR, a);

        const float dx = end.X - forkBase.X;
        const float dy = end.Y - forkBase.Y;
        float nx = -dy;
        float ny = dx;
        const float nl = std::max(1.0f, (float)std::sqrt(nx * nx + ny * ny));
        nx /= nl;
        ny /= nl;

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
            const float tailGate = Smoothstep(0.35f, 1.0f, uu);
            const float aamp = (0.35f + 1.25f * uu) * (0.55f + 0.45f * progress) * tailGate;
            const float ww = (float)sin(timeSec * 7.0f + uu * 9.0f + (seedsCount > 0 ? seeds[(k + i) % seedsCount] : 0.0f));
            const Gdiplus::PointF nn = PolyNormal(raw, k);
            j.push_back(Gdiplus::PointF(raw[k].X + nn.X * ww * aamp, raw[k].Y + nn.Y * ww * aamp));
        }
        out.points = SmoothPolyline(j, 1);
        out.end = end;
        out.angle = a;
        return out;
    };

    const std::vector<BranchGeometry> branches =
        ::mousefx::compute::BuildArray<BranchGeometry>(branchCount, 4, buildBranch);

    for (int i = 0; i < branchCount; ++i) {
        const auto& branch = branches[(size_t)i];
        drawPolyline(branch.points, 4.8f, 0.14f, 1.6f, 0.34f);

        const float dotR = 2.2f + 2.4f * (0.35f + 0.65f * progress);
        {
            Gdiplus::SolidBrush b(MulAlpha(Gdiplus::Color(255, 245, 250, 255), 0.92f * alpha));
            g.FillEllipse(&b, branch.end.X - dotR, branch.end.Y - dotR, dotR * 2.0f, dotR * 2.0f);
        }
        {
            Gdiplus::SolidBrush b(MulAlpha(purple, 0.18f * alpha));
            g.FillEllipse(&b, branch.end.X - dotR * 3.2f, branch.end.Y - dotR * 3.2f, dotR * 6.4f, dotR * 6.4f);
        }

        const float spawnChancePerSec = 0.6f * (0.10f + 0.90f * progress);
        if (rng.Next01() < spawnChancePerSec * dtSec) {
            Spark s;
            s.ang = branch.angle + (rng.Next01() - 0.5f) * 0.35f;
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

    // Keep the tail anchored at 12 o'clock, but avoid a "single hot patch" look:
    // full segment stays faintly readable while the head region is significantly brighter.
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

            const float headBoost = 0.50f + 0.50f * t0;
            const float nearHead = Smoothstep(0.58f, 1.0f, t0);
            const float nearHeadPow = nearHead * nearHead;
            const float baseTail = 0.20f + 0.26f * Smoothstep(0.0f, 0.85f, t0);
            const float focus = (0.12f + 0.76f * headBoost) * nearHeadPow;
            const float a01 = (baseTail + focus) * (0.34f + 0.66f * energyFade);

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

    // Clamp around top while keeping a stronger relation to the head position.
    const float topMin = DegToRad(-145.0f);
    const float topMax = DegToRad(-35.0f);
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
