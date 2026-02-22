#pragma once
#include "../../Interfaces/ITrailRenderer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/TrailColor.h"
#include "MouseFx/Utils/TrailMath.h"
#include "MouseFx/Utils/XorShift.h"
#include <vector>
#include <cmath>
#include <gdiplus.h>
#include <algorithm>

namespace mousefx {

class MeteorRenderer : public ITrailRenderer {
public:
    struct MeteorSpark {
        float x, y;
        float vx, vy;
        float life; // 1.0 to 0.0
        float size;
        float hue;
    };

    MeteorRenderer(int durationMs, const TrailRendererParamsConfig& params)
        : durationMs_(durationMs),
          params_(params.meteor),
          idle_(params.idleFade),
          rng_(prng::Mix32((uint32_t)GetTickCount64())) {}

    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        if (points.empty()) {
            sparks_.clear();
            lastUpdate_ = GetTickCount64();
            return;
        }

        uint64_t now = GetTickCount64();
        int fadeStart = idle_.startMs > 0 ? idle_.startMs : 50;
        int fadeEnd = idle_.endMs > 0 ? idle_.endMs : 260;
        if (fadeEnd <= fadeStart) fadeEnd = fadeStart + 1;
        const float idleFactor = trail_math::IdleFadeFactor(now, points.back().addedTime, fadeStart, fadeEnd);
        float dt = (lastUpdate_ == 0) ? 0.016f : (now - lastUpdate_) / 1000.0f;
        lastUpdate_ = now;

        const auto& head = points.back();
        const ScreenPoint headLocal = ScreenToOverlayPoint(ScreenPoint{head.pt.x, head.pt.y});

        // 1. Emit Sparks if moving
        if (points.size() >= 2) {
            const auto& prev = points[points.size() - 2];
            const ScreenPoint prevLocal = ScreenToOverlayPoint(ScreenPoint{prev.pt.x, prev.pt.y});
            float dx = (float)(headLocal.x - prevLocal.x);
            float dy = (float)(headLocal.y - prevLocal.y);
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1.0f) {
                int emitCount = std::min(4, (int)(dist / 7.0f) + 1);
                emitCount = (int)std::max(1.0f, std::min(8.0f, (float)emitCount * params_.sparkRateScale));
                for (int i = 0; i < emitCount; ++i) {
                    MeteorSpark s;
                    s.x = (float)headLocal.x;
                    s.y = (float)headLocal.y;
                    
                    // Velocity: mostly opposite to move, with some spread
                    float baseAngle = std::atan2(-dy, -dx);
                    float angle = baseAngle + rng_.Range(-0.55f, 0.55f);
                    float speed = rng_.Range(0.6f, 3.2f) * params_.sparkSpeedScale;
                    
                    s.vx = std::cos(angle) * speed;
                    s.vy = std::sin(angle) * speed;
                    s.life = 1.0f;
                    s.size = rng_.Range(1.0f, 3.5f);
                    s.hue = std::fmod((float)now * 0.1f + i * 20.0f, 360.0f);
                    sparks_.push_back(s);
                }
            }
        }

        // 2. Update Sparks
        const float sparkFadeBoost = 1.0f + (1.0f - idleFactor) * 2.0f;
        for (auto it = sparks_.begin(); it != sparks_.end(); ) {
            it->x += it->vx;
            it->y += it->vy;
            it->vy += 0.05f; // Slight gravity
            it->life -= dt * 1.9f * sparkFadeBoost;
            
            if (it->life <= 0) {
                it = sparks_.erase(it);
            } else {
                ++it;
            }
        }

        if (sparks_.size() > 220) {
            sparks_.erase(sparks_.begin(), sparks_.begin() + (sparks_.size() - 220));
        }

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        // 3. Draw Fluid Tail (Spline/Path)
        if (points.size() >= 2) {
            std::vector<Gdiplus::PointF> pathPoints;
            for (const auto& p : points) {
                const ScreenPoint localPt = ScreenToOverlayPoint(ScreenPoint{p.pt.x, p.pt.y});
                pathPoints.push_back(Gdiplus::PointF((float)localPt.x, (float)localPt.y));
            }

            // Draw the tail in segments to allow gradient and width tapering
            for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
                float ratio = (float)i / (float)pathPoints.size(); // 0 at tail, ~1 at head
                
                // Age-based fade
                float age = (float)(now - points[i].addedTime);
                float life = std::max(0.0f, 1.0f - (age / (float)durationMs_));
                life *= idleFactor;
                if (life <= 0) continue;

                float width = 1.0f + 12.0f * ratio * life;
                BYTE alpha = (BYTE)(180 * ratio * life);

                Gdiplus::Color c = color;
                if (isChromatic) {
                    c = trail_color::HslToRgbColor(std::fmod((float)now * 0.15f + (float)i * 8.0f, 360.0f), 0.9f, 0.6f, alpha);
                } else {
                    // Meteor identity: warm tail (less neon-ribbon-like).
                    c = Gdiplus::Color(alpha, 255, 220, 160);
                }

                Gdiplus::Pen pen(c, width);
                pen.SetStartCap(Gdiplus::LineCapRound);
                pen.SetEndCap(Gdiplus::LineCapRound);
                pen.SetLineJoin(Gdiplus::LineJoinRound);

                g.DrawLine(&pen, pathPoints[i], pathPoints[i+1]);
                
                // Add a "core" white line for extra brightness near the head
                if (ratio > 0.6f) {
                    float coreWidth = width * 0.3f;
                    Gdiplus::Pen corePen(Gdiplus::Color((BYTE)(alpha * 0.8f), 255, 255, 255), coreWidth);
                    corePen.SetStartCap(Gdiplus::LineCapRound);
                    corePen.SetEndCap(Gdiplus::LineCapRound);
                    g.DrawLine(&corePen, pathPoints[i], pathPoints[i+1]);
                }
            }
        }

        // 4. Draw Sparks with Glow
        for (const auto& s : sparks_) {
            BYTE alpha = (BYTE)(s.life * 255 * idleFactor);
            Gdiplus::Color sc = isChromatic
                ? trail_color::HslToRgbColor(s.hue, 0.8f, 0.7f, alpha)
                : Gdiplus::Color(alpha, 255, 235, 170);
            
            // Spark core
            Gdiplus::SolidBrush sb(sc);
            g.FillEllipse(&sb, s.x - s.size / 2, s.y - s.size / 2, s.size, s.size);
            
            // Spark soft halo
            if (s.life > 0.5f) {
                 Gdiplus::Color haloC(alpha / 4, sc.GetR(), sc.GetG(), sc.GetB());
                 Gdiplus::SolidBrush hb(haloC);
                 float hs = s.size * 2.5f;
                 g.FillEllipse(&hb, s.x - hs / 2, s.y - hs / 2, hs, hs);
            }
        }

        // 5. Draw Improved Glowing Head
        float headX = (float)headLocal.x;
        float headY = (float)headLocal.y;
        
        // Multi-layered Radial Glow
        struct GlowLayer { float radius; BYTE alpha; Gdiplus::Color color; };
        GlowLayer layers[] = {
            { 18.0f, (BYTE)(40 * idleFactor),  isChromatic ? trail_color::HslToRgbColor(std::fmod((float)now*0.2f, 360.0f), 0.8f, 0.5f, (BYTE)(40 * idleFactor)) : Gdiplus::Color((BYTE)(40 * idleFactor), 255, 200, 120) },
            { 10.0f, (BYTE)(110 * idleFactor), isChromatic ? trail_color::HslToRgbColor(std::fmod((float)now*0.2f, 360.0f), 0.9f, 0.7f, (BYTE)(110 * idleFactor)) : Gdiplus::Color((BYTE)(110 * idleFactor), 255, 240, 180) },
            { 4.0f,  255, Gdiplus::Color(255, 255, 255, 255) } // Bright core
        };

        for (const auto& layer : layers) {
            Gdiplus::SolidBrush brush(layer.color);
            g.FillEllipse(&brush, headX - layer.radius, headY - layer.radius, layer.radius * 2, layer.radius * 2);
        }
        
        // Directional flare (meteor head streak), only while actively moving.
        if (idleFactor > 0.6f && points.size() >= 2) {
            const auto& prev = points[points.size() - 2];
            const ScreenPoint prevLocal = ScreenToOverlayPoint(ScreenPoint{prev.pt.x, prev.pt.y});
            float mdx = (float)(headLocal.x - prevLocal.x);
            float mdy = (float)(headLocal.y - prevLocal.y);
            float mlen = std::sqrt(mdx * mdx + mdy * mdy);
            if (mlen > 0.5f) {
                float inv = 1.0f / mlen;
                float ux = mdx * inv;
                float uy = mdy * inv;
                float px = -uy;
                float py = ux;

                BYTE a = (BYTE)(120 * idleFactor);
                Gdiplus::Pen flarePen(Gdiplus::Color(a, 255, 255, 255), 1.2f);
                float flareLen = 16.0f;
                g.DrawLine(&flarePen, headX - ux * flareLen, headY - uy * flareLen, headX + ux * flareLen * 0.35f, headY + uy * flareLen * 0.35f);
                g.DrawLine(&flarePen, headX - px * 8.0f, headY - py * 8.0f, headX + px * 8.0f, headY + py * 8.0f);
            }
        }
    }

private:
    std::vector<MeteorSpark> sparks_;
    uint64_t lastUpdate_ = 0;
    int durationMs_ = 520;
    MeteorTrailParams params_{};
    IdleFadeParams idle_{};
    prng::XorShift32 rng_;
};

} // namespace mousefx
