#pragma once

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TrailMath.h"
#include <cmath>

namespace mousefx {

class LineTrailRenderer final : public ITrailRenderer {
public:
    explicit LineTrailRenderer(int durationMs = 300) : durationMs_(durationMs) {}

    void Render(Gdiplus::Graphics& g,
                const std::deque<TrailPoint>& points,
                int /*width*/,
                int /*height*/,
                Gdiplus::Color color,
                bool isChromatic) override {
        if (points.size() < 2) return;

        const uint64_t now = GetTickCount64();
        const float idleFactor = trail_math::IdleFadeFactor(now, points.back().addedTime, 60, 220);

        const int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        const int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        for (size_t i = 0; i + 1 < points.size(); ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];

            const uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs_);
            life = trail_math::Clamp01(life) * idleFactor;
            if (life <= 0.0f) continue;

            const int alpha = (int)(life * 255.0f);

            Gdiplus::Color c(alpha, color.GetR(), color.GetG(), color.GetB());
            if (isChromatic) {
                float hue = std::fmod((float)now * 0.10f + (float)i * 12.0f, 360.0f);
                c = trail_color::HslToRgbColor(hue, 0.85f, 0.60f, (BYTE)alpha);
            }

            Gdiplus::Pen pen(c, 4.0f);
            pen.SetStartCap(Gdiplus::LineCapRound);
            pen.SetEndCap(Gdiplus::LineCapRound);
            pen.SetLineJoin(Gdiplus::LineJoinRound);

            g.DrawLine(&pen,
                       (int)p1.pt.x - x_offset,
                       (int)p1.pt.y - y_offset,
                       (int)p2.pt.x - x_offset,
                       (int)p2.pt.y - y_offset);
        }
    }

private:
    int durationMs_ = 300;
};

} // namespace mousefx

