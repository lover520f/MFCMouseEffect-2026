#pragma once

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TrailMath.h"
#include "MouseFx/Utils/TimeUtils.h"
#include <cmath>

namespace mousefx {

class LineTrailRenderer final : public ITrailRenderer {
public:
    LineTrailRenderer(int durationMs, const TrailRendererParamsConfig& params)
        : durationMs_(durationMs), idle_(params.idleFade) {}

    void Render(Gdiplus::Graphics& g,
                const std::deque<TrailPoint>& points,
                int /*width*/,
                int /*height*/,
                Gdiplus::Color color,
                bool isChromatic) override {
        if (points.size() < 2) return;

        const uint64_t now = NowMs();
        int fadeStart = idle_.startMs > 0 ? idle_.startMs : 60;
        int fadeEnd = idle_.endMs > 0 ? idle_.endMs : 220;
        if (fadeEnd <= fadeStart) fadeEnd = fadeStart + 1;
        const float idleFactor = trail_math::IdleFadeFactor(now, points.back().addedTime, fadeStart, fadeEnd);

        for (size_t i = 0; i + 1 < points.size(); ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i + 1];
            const ScreenPoint lp1 = ScreenToOverlayPoint(p1.pt);
            const ScreenPoint lp2 = ScreenToOverlayPoint(p2.pt);

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
                       lp1.x,
                       lp1.y,
                       lp2.x,
                       lp2.y);
        }
    }

private:
    int durationMs_ = 300;
    IdleFadeParams idle_{};
};

} // namespace mousefx
