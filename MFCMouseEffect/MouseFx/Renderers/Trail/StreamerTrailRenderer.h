#pragma once

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TrailMath.h"
#include <algorithm>
#include <cmath>

namespace mousefx {

class StreamerTrailRenderer final : public ITrailRenderer {
public:
    StreamerTrailRenderer(int durationMs, const TrailRendererParamsConfig& params)
        : durationMs_(durationMs), params_(params.streamer), idle_(params.idleFade) {}

    void Render(Gdiplus::Graphics& g,
                const std::deque<TrailPoint>& points,
                int /*width*/,
                int /*height*/,
                Gdiplus::Color color,
                bool isChromatic) override {
        if (points.size() < 2) return;

        const uint64_t now = GetTickCount64();
        int fadeStart = idle_.startMs > 0 ? idle_.startMs : 50;
        int fadeEnd = idle_.endMs > 0 ? idle_.endMs : 260;
        if (fadeEnd <= fadeStart) fadeEnd = fadeStart + 1;
        const float idleFactor = trail_math::IdleFadeFactor(now, points.back().addedTime, fadeStart, fadeEnd);

        const size_t n = points.size();

        // Neon streamer: two-pass stroke (outer glow + inner core) with head-weighted thickness.
        for (size_t i = 0; i + 1 < n; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];
            const POINT lp1 = ScreenToOverlayPoint(p1.pt);
            const POINT lp2 = ScreenToOverlayPoint(p2.pt);

            const float t = (n <= 2) ? 1.0f : (float)i / (float)(n - 1); // 0 tail -> 1 head

            const uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs_);
            life = trail_math::Clamp01(life) * idleFactor;
            if (life <= 0.0f) continue;

            // Strong head, soft tail.
            const float head = std::pow(std::max(0.0f, t), params_.headPower);
            const float w = 2.0f + 18.0f * head * life;

            int alphaCore = (int)(220.0f * head * life);
            int alphaGlow = (int)(90.0f * head * life);
            alphaCore = (int)trail_math::Clamp((float)alphaCore, 0.0f, 255.0f);
            alphaGlow = (int)trail_math::Clamp((float)alphaGlow, 0.0f, 255.0f);

            Gdiplus::Color cCore(alphaCore, color.GetR(), color.GetG(), color.GetB());
            Gdiplus::Color cGlow(alphaGlow, color.GetR(), color.GetG(), color.GetB());

            if (isChromatic) {
                float hue = std::fmod((float)now * 0.18f + (float)i * 6.0f, 360.0f);
                cCore = trail_color::HslToRgbColor(hue, 0.95f, 0.62f, (BYTE)alphaCore);
                cGlow = trail_color::HslToRgbColor(hue, 0.95f, 0.58f, (BYTE)alphaGlow);
            }

            // Outer glow
            {
                Gdiplus::Pen glowPen(cGlow, w * params_.glowWidthScale);
                glowPen.SetStartCap(Gdiplus::LineCapRound);
                glowPen.SetEndCap(Gdiplus::LineCapRound);
                glowPen.SetLineJoin(Gdiplus::LineJoinRound);
                g.DrawLine(&glowPen,
                           lp1.x,
                           lp1.y,
                           lp2.x,
                           lp2.y);
            }

            // Inner core
            {
                Gdiplus::Pen corePen(cCore, std::max(1.5f, w * params_.coreWidthScale));
                corePen.SetStartCap(Gdiplus::LineCapRound);
                corePen.SetEndCap(Gdiplus::LineCapRound);
                corePen.SetLineJoin(Gdiplus::LineJoinRound);
                g.DrawLine(&corePen,
                           lp1.x,
                           lp1.y,
                           lp2.x,
                           lp2.y);
            }
        }
    }

private:
    int durationMs_ = 420;
    StreamerTrailParams params_{};
    IdleFadeParams idle_{};
};

} // namespace mousefx
