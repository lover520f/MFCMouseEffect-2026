#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace mousefx {

class HologramHudRenderer : public IRippleRenderer {
public:
    struct Vec3 { float x, y, z; };
    struct Particle { Vec3 pos; float speed; float life; };
    std::vector<Particle> particles_;

    // Project 3D point to 2D
    static Gdiplus::PointF Project(const Vec3& v, float cx, float cy, float tiltRad) {
        float cosT = cos(tiltRad);
        float sinT = sin(tiltRad);
        // Rotate X (Tilt)
        float y_rot = v.y * cosT - v.z * sinT;
        float z_rot = v.y * sinT + v.z * cosT;
        float x_rot = v.x;
        // Perspective
        float dist = 400.0f;
        float scale = dist / (dist + z_rot);
        return Gdiplus::PointF(cx + x_rot * scale, cy + y_rot * scale);
    }

    void Start(const RippleStyle& style) override {
        particles_.clear();
        particles_.reserve(30);
        for(int i=0; i<30; ++i) {
            float angle = ((float)(rand() % 360) / 180.0f) * 3.14159f;
            float r = style.endRadius * (0.2f + (float)(rand()%80)/100.0f);
            particles_.push_back({ {r * cos(angle), 0.0f, r * sin(angle)}, 0.5f + (float)(rand()%100)/50.0f, 1.0f });
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;
        const float radius = style.endRadius;
        const float progress = Clamp01(t); 
        float timeSec = (float)elapsedMs / 1000.0f;
        float tilt = 50.0f * 3.14159f / 180.0f; // Less steep tilt

        Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        
        // 1. Draw Ground Ring Segments (tech look)
        auto DrawSegmentRing = [&](float r, float spin, float width, int count, float gapRatio) {
             constexpr int segs = 10;
             for (int i = 0; i < count; ++i) {
                 float step = 2 * 3.14159f / count;
                 float startAng = i * step + spin;
                 float sweep = step * (1.0f - gapRatio);

                 segmentPointsScratch_.clear();
                 segmentPointsScratch_.reserve(segs + 1);
                 for(int j=0; j<=segs; ++j) {
                     float ang = startAng + sweep * ((float)j/segs);
                     segmentPointsScratch_.push_back(Project({r * cos(ang), 0.0f, r * sin(ang)}, cx, cy, tilt));
                 }

                 BYTE a = ClampByte((int)(stroke.GetA() * (1.0f - progress * 0.5f))); 
                 Gdiplus::Pen p(Gdiplus::Color(a, stroke.GetR(), stroke.GetG(), stroke.GetB()), width);
                 g.DrawLines(&p, segmentPointsScratch_.data(), (INT)segmentPointsScratch_.size());
             }
        };

        float rMain = radius * (0.3f + 0.7f * progress);
        DrawSegmentRing(rMain, timeSec * 0.5f, 4.0f, 3, 0.2f);
        DrawSegmentRing(rMain * 0.7f, -timeSec, 2.0f, 4, 0.4f);

        // 2. Rising Particles
        if(particles_.empty()) Start(style);
        
        BYTE pAlpha = ClampByte((int)(stroke.GetA() * 0.7f));
        Gdiplus::SolidBrush pb(Gdiplus::Color(pAlpha, stroke.GetR(), stroke.GetG(), stroke.GetB()));
        
        for(auto& p : particles_) {
            p.pos.y -= p.speed * 2.0f; 
            if(p.pos.y < -radius * 1.5f) {
                p.pos.y = 0; 
                float angle = ((float)(rand() % 360) / 180.0f) * 3.14159f;
                float r = radius * (0.2f + (float)(rand()%80)/100.0f);
                p.pos.x = r * cos(angle);
                p.pos.z = r * sin(angle);
            }
            
            Gdiplus::PointF pt = Project(p.pos, cx, cy, tilt);
            float distScale = 400.0f / (400.0f + p.pos.z);
            float pSize = 3.0f * distScale;
            g.FillEllipse(&pb, pt.X - pSize/2, pt.Y - pSize/2, pSize, pSize);
            
            Gdiplus::Pen tracePen(Gdiplus::Color(ClampByte((int)(pAlpha*0.3f)), stroke.GetR(), stroke.GetG(), stroke.GetB()), 1.0f);
            Gdiplus::PointF groundPt = Project({p.pos.x, 0, p.pos.z}, cx, cy, tilt);
            g.DrawLine(&tracePen, pt, groundPt);
        }

        // 3. Central Core
        {
            Gdiplus::GraphicsPath path;
            Gdiplus::PointF center = Project({0,0,0}, cx, cy, tilt);
            float coreR = style.strokeWidth * 4.0f;
            path.AddEllipse(center.X - coreR, center.Y - coreR, coreR*2, coreR*2);
            
            Gdiplus::PathGradientBrush pgb(&path);
            pgb.SetCenterPoint(center);
            pgb.SetCenterColor(Gdiplus::Color(0, 255, 255, 255));
            Gdiplus::Color surround[] = { Gdiplus::Color(0, stroke.GetR(), stroke.GetG(), stroke.GetB()) };
            int n = 1;
            pgb.SetSurroundColors(surround, &n);
            g.FillPath(&pgb, &path);
        }
    }

private:
    std::vector<Gdiplus::PointF> segmentPointsScratch_;
};

REGISTER_RENDERER("hologram", HologramHudRenderer)
// Alias
// REGISTER_RENDERER("scifi3d", HologramHudRenderer)

} // namespace mousefx
