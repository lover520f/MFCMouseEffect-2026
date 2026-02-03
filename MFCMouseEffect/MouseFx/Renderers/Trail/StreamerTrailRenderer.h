#pragma once

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TrailMath.h"
#include <algorithm>
#include <cmath>

namespace mousefx {

class StreamerTrailRenderer final : public ITrailRenderer {
public:
    explicit StreamerTrailRenderer(int durationMs = 420) : durationMs_(durationMs) {}

    void Render(Gdiplus::Graphics& g,
                const std::deque<TrailPoint>& points,
                int /*width*/,
                int /*height*/,
                Gdiplus::Color color,
                bool isChromatic) override {
        if (points.size() < 2) return;

        const uint64_t now = GetTickCount64();
        const float idleFactor = trail_math::IdleFadeFactor(now, points.back().addedTime, 50, 260);

        const int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        const int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        const size_t n = points.size();

        // Neon streamer: two-pass stroke (outer glow + inner core) with head-weighted thickness.
        for (size_t i = 0; i + 1 < n; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];

            const float t = (n <= 2) ? 1.0f : (float)i / (float)(n - 1); // 0 tail -> 1 head

            const uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs_);
            life = trail_math::Clamp01(life) * idleFactor;
            if (life <= 0.0f) continue;

            // Strong head, soft tail.
            const float head = std::pow(std::max(0.0f, t), 1.6f);
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
                Gdiplus::Pen glowPen(cGlow, w * 1.8f);
                glowPen.SetStartCap(Gdiplus::LineCapRound);
                glowPen.SetEndCap(Gdiplus::LineCapRound);
                glowPen.SetLineJoin(Gdiplus::LineJoinRound);
                g.DrawLine(&glowPen,
                           (int)p1.pt.x - x_offset,
                           (int)p1.pt.y - y_offset,
                           (int)p2.pt.x - x_offset,
                           (int)p2.pt.y - y_offset);
            }

            // Inner core
            {
                Gdiplus::Pen corePen(cCore, std::max(1.5f, w * 0.55f));
                corePen.SetStartCap(Gdiplus::LineCapRound);
                corePen.SetEndCap(Gdiplus::LineCapRound);
                corePen.SetLineJoin(Gdiplus::LineJoinRound);
                g.DrawLine(&corePen,
                           (int)p1.pt.x - x_offset,
                           (int)p1.pt.y - y_offset,
                           (int)p2.pt.x - x_offset,
                           (int)p2.pt.y - y_offset);
            }
        }
    }

private:
    int durationMs_ = 420;
};

} // namespace mousefx
