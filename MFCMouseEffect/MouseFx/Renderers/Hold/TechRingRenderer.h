#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace mousefx {

// Uses the Gyro/Hologram style
class TechRingRenderer : public IRippleRenderer {
public:
    struct Vec3 { float x, y, z; };
    struct Particle { Vec3 pos; float speed; float life; };
    std::vector<Particle> particles_;

    // 3D Rotation Helpers
    static Vec3 RotX(const Vec3& v, float angle) {
        float c = cos(angle), s = sin(angle);
        return { v.x, v.y * c - v.z * s, v.y * s + v.z * c };
    }
    static Vec3 RotY(const Vec3& v, float angle) {
        float c = cos(angle), s = sin(angle);
        return { v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
    }
    static Vec3 RotZ(const Vec3& v, float angle) {
        float c = cos(angle), s = sin(angle);
        return { v.x * c - v.y * s, v.x * s + v.y * c, v.z };
    }

    // Project 3D point to 2D
    static Gdiplus::PointF Project(const Vec3& v, float cx, float cy) {
        float dist = 500.0f; // Camera distance
        float scale = dist / (dist + v.z); 
        return Gdiplus::PointF(cx + v.x * scale, cy + v.y * scale);
    }

    void Start(const RippleStyle& style) override {
        particles_.clear();
        particles_.reserve(40);
        for(int i=0; i<40; ++i) {
            float theta = ((float)(rand() % 360) / 180.0f) * 3.14159f;
            float phi = ((float)(rand() % 180) / 180.0f) * 3.14159f;
            float r = style.endRadius * 0.5f * ((float)(rand() % 100) / 100.0f);
            
            Vec3 p;
            p.x = r * sin(phi) * cos(theta);
            p.y = r * sin(phi) * sin(theta);
            p.z = r * cos(phi);
            
            particles_.push_back({ p, 1.0f + (float)(rand()%100)/50.0f, 1.0f });
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;
        const float radius = style.endRadius * (0.8f + 0.2f * Clamp01(t)); // Slight expansion
        const float progress = Clamp01(t); 
        float timeSec = (float)elapsedMs / 1000.0f;
        
        float baseTiltX = 20.0f * 3.14159f / 180.0f; 
        float baseTiltY = 10.0f * 3.14159f / 180.0f;

        Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        Gdiplus::Color glow = ToGdiPlus(style.glow);
        Gdiplus::Color fill = ToGdiPlus(style.fill);
        (void)glow;

        auto DrawRing3D = [&](float r, float spin, float width, Gdiplus::Color color, float startArc, float sweepArc, int axisMode, float axisTilt) {
             constexpr int segments = 60;
             ringPointsScratch_.clear();
             ringPointsScratch_.reserve(segments + 1);
             for (int i = 0; i <= segments; ++i) {
                 float pct = (float)i / segments;
                 float ang = startArc + sweepArc * pct;
                 
                 Vec3 v = { r * cos(ang), r * sin(ang), 0.0f };
                 
                 if (axisMode == 0) v = RotZ(v, spin);
                 else if (axisMode == 1) { v = RotZ(v, spin); v = RotX(v, axisTilt); }
                 else if (axisMode == 2) { v = RotZ(v, spin); v = RotY(v, axisTilt); }
                 
                 v = RotX(v, baseTiltX);
                 v = RotY(v, baseTiltY);
                 
                 ringPointsScratch_.push_back(Project(v, cx, cy));
             }
             
             Gdiplus::Pen p(color, width);
             if (sweepArc < 6.28f) p.SetStartCap(Gdiplus::LineCapRound), p.SetEndCap(Gdiplus::LineCapRound);
             g.DrawLines(&p, ringPointsScratch_.data(), (INT)ringPointsScratch_.size());
        };

        // 1. Core Energy Orb
        {
            float coreR = style.strokeWidth * 6.0f;
            Gdiplus::GraphicsPath path;
            Vec3 center3D = {0,0,0};
            Gdiplus::PointF center = Project(center3D, cx, cy);
            
            path.AddEllipse(center.X - coreR, center.Y - coreR, coreR*2, coreR*2);
            Gdiplus::PathGradientBrush pgb(&path);
            
            // Reduced Alpha for center to show text behind (25 -> 0) - removed completely per user request
            pgb.SetCenterColor(Gdiplus::Color(0, 220, 240, 255)); 
            Gdiplus::Color surround[] = { Gdiplus::Color(0, stroke.GetR(), stroke.GetG(), stroke.GetB()) };
            int n = 1;
            pgb.SetSurroundColors(surround, &n);
            g.FillPath(&pgb, &path);
        }

        // 2. Inner Gyro Ring
        {
             BYTE a = ClampByte((int)(stroke.GetA() * 0.5f));
             Gdiplus::Color c(a, stroke.GetR(), stroke.GetG(), stroke.GetB());
             DrawRing3D(radius * 0.6f, timeSec * 1.5f, 2.0f, c, 0, 6.28f, 1, 60.0f * 3.14159f / 180.0f);
        }

        // 3. Middle Gyro Ring
        {
             BYTE a = ClampByte((int)(stroke.GetA() * 0.4f));
             Gdiplus::Color c(a, fill.GetR(), fill.GetG(), fill.GetB());
             DrawRing3D(radius * 0.8f, -timeSec * 1.2f, 2.0f, c, 0, 6.28f, 2, 45.0f * 3.14159f / 180.0f);
        }

        // 4. Outer Scanner Ring
        {
             float sweep = 6.28f * 0.75f * progress; 
             float spin = timeSec * 0.8f;
             
             BYTE a = ClampByte((int)(stroke.GetA() * 0.7f));
             Gdiplus::Color c(a, stroke.GetR(), stroke.GetG(), stroke.GetB());
             
             DrawRing3D(radius * 1.1f, spin, 3.0f * style.strokeWidth, c, 0.0f, sweep, 0, 0);
             
             DrawRing3D(radius * 1.15f, -spin * 1.5f, 1.0f, c, 0.0f, 2.0f, 0, 0); 
        }
        
        // 5. Particles
        if(particles_.empty()) Start(style);
        
        Gdiplus::SolidBrush pb(Gdiplus::Color(120, stroke.GetR(), stroke.GetG(), stroke.GetB()));
        for(auto& p : particles_) {
            Vec3 pos = p.pos;
            pos = RotY(pos, timeSec * 0.5f);
            pos = RotX(pos, baseTiltX);
            
            Gdiplus::PointF pt = Project(pos, cx, cy);
            float distScale = 500.0f / (500.0f + pos.z);
            float pSize = 3.0f * distScale;
            
            g.FillEllipse(&pb, pt.X - pSize/2, pt.Y - pSize/2, pSize, pSize);
        }
    }

private:
    std::vector<Gdiplus::PointF> ringPointsScratch_;
};

REGISTER_RENDERER("tech_ring", TechRingRenderer)

} // namespace mousefx
