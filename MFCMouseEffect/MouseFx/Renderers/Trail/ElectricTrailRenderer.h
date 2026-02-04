#pragma once

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TrailMath.h"
#include "MouseFx/Utils/XorShift.h"
#include <algorithm>
#include <cmath>

namespace mousefx {

class ElectricTrailRenderer final : public ITrailRenderer {
public:
    ElectricTrailRenderer(int durationMs, const TrailRendererParamsConfig& params)
        : durationMs_(durationMs), params_(params.electric), idle_(params.idleFade) {}

    void Render(Gdiplus::Graphics& g,
                const std::deque<TrailPoint>& points,
                int /*width*/,
                int /*height*/,
                Gdiplus::Color color,
                bool isChromatic) override {
        if (points.size() < 2) return;

        const uint64_t now = GetTickCount64();
        const uint64_t frameKey = now / 30; // stabilize jitter within ~1-2 frames
        int fadeStart = idle_.startMs > 0 ? idle_.startMs : 40;
        int fadeEnd = idle_.endMs > 0 ? idle_.endMs : 180;
        if (fadeEnd <= fadeStart) fadeEnd = fadeStart + 1;
        const float idleFactor = trail_math::IdleFadeFactor(now, points.back().addedTime, fadeStart, fadeEnd);

        const int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        const int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        for (size_t i = 0; i + 1 < points.size(); ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];

            const uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs_);
            life = trail_math::Clamp01(life) * idleFactor;
            if (life <= 0.0f) continue;

            // Seed per segment + frame bucket for animated but stable arcs.
            uint32_t seed = prng::Mix32((uint32_t)frameKey ^ (uint32_t)(i * 0x9E3779B9u));
            prng::XorShift32 rng(seed);

            float x1 = (float)p1.pt.x - (float)x_offset;
            float y1 = (float)p1.pt.y - (float)y_offset;
            float x2 = (float)p2.pt.x - (float)x_offset;
            float y2 = (float)p2.pt.y - (float)y_offset;

            float dx = x2 - x1;
            float dy = y2 - y1;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len < 0.5f) continue;

            float invLen = 1.0f / len;
            float nx = -dy * invLen;
            float ny = dx * invLen;

            float amp = std::min(10.0f, std::max(2.0f, len * 0.12f)) * life * params_.amplitudeScale;
            float o1 = rng.Range(-1.0f, 1.0f) * amp;
            float o2 = rng.Range(-1.0f, 1.0f) * amp;

            Gdiplus::PointF a(x1, y1);
            Gdiplus::PointF b(x1 + dx * 0.35f + nx * o1, y1 + dy * 0.35f + ny * o1);
            Gdiplus::PointF c(x1 + dx * 0.70f + nx * o2, y1 + dy * 0.70f + ny * o2);
            Gdiplus::PointF d(x2, y2);

            const float coreW = std::max(1.0f, 2.2f * life);
            const float glowW = coreW * 3.0f;

            int alpha = (int)trail_math::Clamp(255.0f * life, 0.0f, 255.0f);

            Gdiplus::Color cGlow(alpha / 2, 70, 235, 255);
            Gdiplus::Color cCore(alpha, 255, 255, 255);

            if (isChromatic) {
                float hue = std::fmod((float)now * 0.55f + (float)i * 18.0f, 360.0f);
                cGlow = trail_color::HslToRgbColor(hue, 1.0f, 0.60f, (BYTE)(alpha / 2));
                cCore = trail_color::HslToRgbColor(std::fmod(hue + 30.0f, 360.0f), 1.0f, 0.75f, (BYTE)alpha);
            } else {
                // Let theme tint the glow slightly, but keep a white core for "electric" identity.
                cGlow = Gdiplus::Color(alpha / 2,
                                       (BYTE)std::max<int>(70, color.GetR()),
                                       (BYTE)std::max<int>(200, color.GetG()),
                                       (BYTE)std::max<int>(220, color.GetB()));
            }

            // Glow stroke
            {
                Gdiplus::Pen penGlow(cGlow, glowW);
                penGlow.SetLineJoin(Gdiplus::LineJoinRound);
                penGlow.SetStartCap(Gdiplus::LineCapRound);
                penGlow.SetEndCap(Gdiplus::LineCapRound);
                g.DrawLine(&penGlow, a, b);
                g.DrawLine(&penGlow, b, c);
                g.DrawLine(&penGlow, c, d);
            }

            // Core stroke
            {
                Gdiplus::Pen penCore(cCore, coreW);
                penCore.SetLineJoin(Gdiplus::LineJoinRound);
                penCore.SetStartCap(Gdiplus::LineCapRound);
                penCore.SetEndCap(Gdiplus::LineCapRound);
                g.DrawLine(&penCore, a, b);
                g.DrawLine(&penCore, b, c);
                g.DrawLine(&penCore, c, d);
            }

            // Occasional fork to make it feel less like a "jittery ribbon".
            float forkChance = params_.forkChance * life;
            if (rng.Next01() < forkChance) {
                float t = rng.Range(0.35f, 0.75f);
                Gdiplus::PointF base(x1 + dx * t, y1 + dy * t);
                float forkLen = rng.Range(10.0f, 22.0f) * life;
                float forkSide = (rng.Next01() < 0.5f) ? -1.0f : 1.0f;
                Gdiplus::PointF tip(base.X + nx * forkSide * forkLen + dx * 0.05f, base.Y + ny * forkSide * forkLen + dy * 0.05f);

                Gdiplus::Pen forkPen(cGlow, std::max(1.0f, coreW * 1.2f));
                forkPen.SetLineJoin(Gdiplus::LineJoinRound);
                forkPen.SetStartCap(Gdiplus::LineCapRound);
                forkPen.SetEndCap(Gdiplus::LineCapRound);
                g.DrawLine(&forkPen, base, tip);
            }
        }
    }

private:
    int durationMs_ = 280;
    ElectricTrailParams params_{};
    IdleFadeParams idle_{};
};

} // namespace mousefx
