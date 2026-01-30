#pragma once
#include "../../Interfaces/ITrailRenderer.h"
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

     MeteorRenderer() {
        srand((unsigned int)GetTickCount64());
    }

    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        if (points.empty()) {
            sparks_.clear();
            lastUpdate_ = GetTickCount64();
            return;
        }

        uint64_t now = GetTickCount64();
        float dt = (lastUpdate_ == 0) ? 0.016f : (now - lastUpdate_) / 1000.0f;
        lastUpdate_ = now;

        const auto& head = points.back();
        int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        // 1. Emit Sparks if moving
        if (points.size() >= 2) {
            const auto& prev = points[points.size() - 2];
            float dx = (float)(head.pt.x - prev.pt.x);
            float dy = (float)(head.pt.y - prev.pt.y);
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 1.0f) {
                int emitCount = std::min(3, (int)(dist / 8.0f) + 1);
                for (int i = 0; i < emitCount; ++i) {
                    MeteorSpark s;
                    s.x = (float)head.pt.x - x_offset;
                    s.y = (float)head.pt.y - y_offset;
                    
                    // Velocity: mostly opposite to move, with some spread
                    float baseAngle = std::atan2(-dy, -dx);
                    float angle = baseAngle + ((rand() % 60 - 30) * 3.14159f / 180.0f);
                    float speed = (float)(rand() % 100) / 40.0f + 0.5f;
                    
                    s.vx = std::cos(angle) * speed;
                    s.vy = std::sin(angle) * speed;
                    s.life = 1.0f;
                    s.size = (float)(rand() % 25) / 10.0f + 1.0f;
                    s.hue = std::fmod((float)now * 0.1f + i * 20.0f, 360.0f);
                    sparks_.push_back(s);
                }
            }
        }

        // 2. Update Sparks
        for (auto it = sparks_.begin(); it != sparks_.end(); ) {
            it->x += it->vx;
            it->y += it->vy;
            it->vy += 0.05f; // Slight gravity
            it->life -= dt * 1.8f; 
            
            if (it->life <= 0) {
                it = sparks_.erase(it);
            } else {
                ++it;
            }
        }

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        // 3. Draw Fluid Tail (Spline/Path)
        if (points.size() >= 2) {
            std::vector<Gdiplus::PointF> pathPoints;
            for (const auto& p : points) {
                pathPoints.push_back(Gdiplus::PointF((float)p.pt.x - x_offset, (float)p.pt.y - y_offset));
            }

            // Draw the tail in segments to allow gradient and width tapering
            for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
                float ratio = (float)i / (float)pathPoints.size(); // 0 at tail, ~1 at head
                
                // Age-based fade
                float age = (float)(now - points[i].addedTime);
                float life = std::max(0.0f, 1.0f - (age / 450.0f));
                if (life <= 0) continue;

                float width = 1.0f + 12.0f * ratio * life;
                BYTE alpha = (BYTE)(180 * ratio * life);

                Gdiplus::Color c = color;
                if (isChromatic) {
                    c = HslToRgb(std::fmod((float)now * 0.15f + i * 8.0f, 360.0f), 0.9f, 0.6f, alpha);
                } else {
                    c = Gdiplus::Color(alpha, color.GetR(), color.GetG(), color.GetB());
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
            BYTE alpha = (BYTE)(s.life * 255);
            Gdiplus::Color sc = isChromatic ? HslToRgb(s.hue, 0.8f, 0.7f, alpha) : Gdiplus::Color(alpha, 255, 240, 150);
            
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
        float headX = (float)head.pt.x - x_offset;
        float headY = (float)head.pt.y - y_offset;
        
        // Multi-layered Radial Glow
        struct GlowLayer { float radius; BYTE alpha; Gdiplus::Color color; };
        GlowLayer layers[] = {
            { 18.0f, 40,  isChromatic ? HslToRgb(std::fmod((float)now*0.2f, 360.0f), 0.8f, 0.5f, 40) : Gdiplus::Color(40, color.GetR(), color.GetG(), color.GetB()) },
            { 10.0f, 100, isChromatic ? HslToRgb(std::fmod((float)now*0.2f, 360.0f), 0.9f, 0.7f, 100) : Gdiplus::Color(100, 255, 255, 200) },
            { 4.0f,  255, Gdiplus::Color(255, 255, 255, 255) } // Bright core
        };

        for (const auto& layer : layers) {
            Gdiplus::SolidBrush brush(layer.color);
            g.FillEllipse(&brush, headX - layer.radius, headY - layer.radius, layer.radius * 2, layer.radius * 2);
        }
        
        // Add a "Cross" or "Star" flare effect occasionally or sublty
        Gdiplus::Pen flarePen(Gdiplus::Color(150, 255, 255, 255), 1.5f);
        float flareLen = 12.0f;
        g.DrawLine(&flarePen, headX - flareLen, headY, headX + flareLen, headY);
        g.DrawLine(&flarePen, headX, headY - flareLen, headX, headY + flareLen);
    }

private:
     Gdiplus::Color HslToRgb(float h, float s, float l, BYTE alpha) {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        auto hue2rgb = [](float p, float q, float t) {
            if (t < 0.0f) t += 1.0f;
            if (t > 1.0f) t -= 1.0f;
            if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
            if (t < 1.0f / 2.0f) return q;
            if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        };
        float tr = h / 360.0f + 1.0f / 3.0f;
        float tg = h / 360.0f;
        float tb = h / 360.0f - 1.0f / 3.0f;
        return Gdiplus::Color(alpha, (BYTE)(hue2rgb(p, q, tr) * 255), (BYTE)(hue2rgb(p, q, tg) * 255), (BYTE)(hue2rgb(p, q, tb) * 255));
    }

    std::vector<MeteorSpark> sparks_;
    uint64_t lastUpdate_ = 0;
};

} // namespace mousefx
