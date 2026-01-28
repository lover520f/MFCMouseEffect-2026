#pragma once

#include "IRippleRenderer.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace mousefx {

// Helper functions (formerly static in RippleWindow.cpp)
namespace render_utils {
    inline float Clamp01(float v) { return (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v); }
    inline int ClampInt(int v, int lo, int hi) { if (v < lo) return lo; if (v > hi) return hi; return v; }
    inline BYTE ClampByte(int v) { return (BYTE)ClampInt(v, 0, 255); }
    
    inline Gdiplus::Color ToGdiPlus(const Argb& c) {
        return Gdiplus::Color((BYTE)((c.value >> 24) & 0xFF), 
                              (BYTE)((c.value >> 16) & 0xFF), 
                              (BYTE)((c.value >> 8) & 0xFF), 
                              (BYTE)(c.value & 0xFF));
    }

    inline float fnmod(float x, float y) {
        return x - y * floor(x / y);
    }
}

// 1. Charge Ring Renderer (Original "Charge")
class ChargeRenderer : public IRippleRenderer {
public:
    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;
        const float radius = style.endRadius;
        const float progress = Clamp01(t);
        const float pulse = 0.5f + 0.5f * (float)sin((double)elapsedMs * 0.0045);
        const float alpha = 0.55f + 0.45f * pulse;

        const Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        const BYTE aStroke = ClampByte((int)(stroke.GetA() * alpha));
        const Gdiplus::Color glow = ToGdiPlus(style.glow);
        const BYTE aGlow = ClampByte((int)(glow.GetA() * alpha * 0.6f));

        // Background ring
        Gdiplus::Pen bgPen(Gdiplus::Color(ClampByte((int)(aStroke * 0.25f)),
            stroke.GetR(), stroke.GetG(), stroke.GetB()), style.strokeWidth);
        bgPen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawEllipse(&bgPen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        // Glow arc
        Gdiplus::Pen glowPen(Gdiplus::Color(aGlow, glow.GetR(), glow.GetG(), glow.GetB()),
            style.strokeWidth + 8.0f);
        glowPen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawArc(&glowPen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, -90.0f, progress * 360.0f);

        // Progress arc
        Gdiplus::Pen pen(Gdiplus::Color(aStroke, stroke.GetR(), stroke.GetG(), stroke.GetB()),
            style.strokeWidth + 1.0f);
        pen.SetLineJoin(Gdiplus::LineJoinRound);
        g.DrawArc(&pen, cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, -90.0f, progress * 360.0f);

        // Accent dot at head
        const float angle = (-90.0f + progress * 360.0f) * 3.1415926f / 180.0f;
        const float dotX = cx + (float)cos(angle) * radius;
        const float dotY = cy + (float)sin(angle) * radius;
        const float dotR = style.strokeWidth * 2.2f;
        Gdiplus::SolidBrush dotBrush(Gdiplus::Color(aStroke, stroke.GetR(), stroke.GetG(), stroke.GetB()));
        g.FillEllipse(&dotBrush, dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
    }
};

// 2. Lightning Renderer
class LightningRenderer : public IRippleRenderer {
public:
    struct Particle {
        float angle;
        float speed;
        float phase;
        float distOffset;
        bool active;
    };
    std::vector<Particle> particles_;

    void Start(const RippleStyle& style) override {
        // Initialize particles
        particles_.clear();
        for (int i = 0; i < 24; ++i) {
            Particle p;
            p.angle = ((float)(rand() % 360) / 180.0f) * 3.14159f;
            p.speed = 1.0f + ((float)(rand() % 100) / 50.0f); // 1.0 - 3.0
            p.phase = ((float)(rand() % 100) / 100.0f);
            p.distOffset = ((float)(rand() % 100) / 100.0f) * 5.0f;
            p.active = true;
            particles_.push_back(p);
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;
        const float radius = style.endRadius;
        
        // Central Core
        // Grows as t increases to simulate energy buildup
        float coreRadius = style.strokeWidth * (2.0f + t * 4.0f); // 2x -> 6x
        // Pulse the core
        float pulse = 0.5f + 0.5f * (float)sin((double)elapsedMs * 0.05); // Fast pulse
        float coreAlpha = 0.8f + 0.2f * pulse;
        
        Gdiplus::Color glow = ToGdiPlus(style.glow);
        Gdiplus::Color coreColor = ToGdiPlus(style.stroke);
        
        // Core Glow
        {
             Gdiplus::GraphicsPath path;
             path.AddEllipse(cx - coreRadius * 2.5f, cy - coreRadius * 2.5f, coreRadius * 5.0f, coreRadius * 5.0f);
             Gdiplus::PathGradientBrush pgb(&path);
             pgb.SetCenterColor(Gdiplus::Color(ClampByte((int)(glow.GetA() * 0.6f)), glow.GetR(), glow.GetG(), glow.GetB()));
             Gdiplus::Color surround[1] = { Gdiplus::Color(0, glow.GetR(), glow.GetG(), glow.GetB()) };
             int count = 1;
             pgb.SetSurroundColors(surround, &count);
             pgb.SetCenterPoint(Gdiplus::PointF(cx, cy));
             g.FillPath(&pgb, &path);
        }

        // Draw Core (Optimized: Energy Orb instead of solid blob)
        {
             Gdiplus::GraphicsPath path;
             // Slightly larger than before to account for soft edges
             float drawRadius = coreRadius * 1.5f; 
             path.AddEllipse(cx - drawRadius, cy - drawRadius, drawRadius * 2.0f, drawRadius * 2.0f);
             
             Gdiplus::PathGradientBrush pgb(&path);
             
             // Center: Very bright (almost white), high alpha
             // Blend coreColor with White for the center hot spot
             BYTE r = (BYTE)std::min(255, (int)coreColor.GetR() + 100);
             BYTE gVal = (BYTE)std::min(255, (int)coreColor.GetG() + 100);
             BYTE bVal = (BYTE)std::min(255, (int)coreColor.GetB() + 100);
             
             pgb.SetCenterColor(Gdiplus::Color(ClampByte((int)(coreAlpha * 255)), r, gVal, bVal));
             
             // Edge: Theme color but transparent
             Gdiplus::Color surround[1] = { Gdiplus::Color(0, coreColor.GetR(), coreColor.GetG(), coreColor.GetB()) };
             int count = 1;
             pgb.SetSurroundColors(surround, &count);
             pgb.SetCenterPoint(Gdiplus::PointF(cx, cy));
             
             // Focus scale to make the hot spot smaller
             pgb.SetFocusScales(0.4f, 0.4f); 
             
             g.FillPath(&pgb, &path);
        }
        
        // Draw Particles / Lightning streaks
        srand((unsigned int)elapsedMs); // Deterministic-ish for frame but we want jitter
        // Actually earlier code used startTick, but here we don't have startTick easily unless passed.
        // Using elapsedMs preserves jitter per frame.
        
        Gdiplus::Pen boltPen(Gdiplus::Color(ClampByte((int)(coreColor.GetA() * 0.7f)), coreColor.GetR(), coreColor.GetG(), coreColor.GetB()), style.strokeWidth * 0.5f);
        
        for (auto& p : particles_) {
            float linearProgress = (fnmod((float)elapsedMs * 0.001f * p.speed + p.phase, 1.0f)); 
            float distFactor = 1.0f - linearProgress;
            
            float currentDist = radius * 0.3f + (radius * 0.8f) * distFactor + p.distOffset * 10.0f;
            if (currentDist > radius) currentDist = radius; 
            
            float angle = p.angle;
            
            float length = 15.0f * (1.0f + p.speed);
            
            float x1 = cx + (float)cos(angle) * currentDist;
            float y1 = cy + (float)sin(angle) * currentDist;
            float x2 = cx + (float)cos(angle) * (currentDist + length);
            float y2 = cy + (float)sin(angle) * (currentDist + length);
            
            float jitter = 2.0f * ((float)(rand() % 100) / 100.0f - 0.5f);
            
            g.DrawLine(&boltPen, x1 + jitter, y1 + jitter, x2, y2);
        }
    }
};

// 3. Hex Renderer
class HexRenderer : public IRippleRenderer {
public:
    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;

        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;
        const float radius = style.endRadius; // Base radius
        const float progress = Clamp01(t); 
        
        const float pulse = 0.5f + 0.5f * (float)sin((double)elapsedMs * 0.003);
        const float alphaBase = 0.6f + 0.4f * pulse;
        
        Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        Gdiplus::Color glow = ToGdiPlus(style.glow);
        Gdiplus::Color fill = ToGdiPlus(style.fill); 
        
        auto DrawHex = [&](float r, float rotRad, float width, float aFactor, bool fillHex) {
            Gdiplus::PointF pts[6];
            for (int i = 0; i < 6; ++i) {
                float angle = rotRad + (float)i * (3.14159f / 3.0f);
                pts[i].X = cx + r * (float)cos(angle);
                pts[i].Y = cy + r * (float)sin(angle);
            }
            
            BYTE a = ClampByte((int)(stroke.GetA() * alphaBase * aFactor));
            Gdiplus::Color c(a, stroke.GetR(), stroke.GetG(), stroke.GetB());
            
            if (fillHex) {
                 BYTE fa = ClampByte((int)(fill.GetA() * alphaBase * aFactor * 0.3f));
                 Gdiplus::SolidBrush b(Gdiplus::Color(fa, fill.GetR(), fill.GetG(), fill.GetB()));
                 g.FillPolygon(&b, pts, 6);
            }

            Gdiplus::Pen p(c, width);
            p.SetLineJoin(Gdiplus::LineJoinMiter);
            g.DrawPolygon(&p, pts, 6);
            
            BYTE ga = ClampByte((int)(glow.GetA() * alphaBase * aFactor));
            Gdiplus::SolidBrush gb(Gdiplus::Color(ga, glow.GetR(), glow.GetG(), glow.GetB()));
            float dotR = width * 1.5f;
            for(int i=0; i<6; ++i) {
                g.FillEllipse(&gb, pts[i].X - dotR, pts[i].Y - dotR, dotR*2, dotR*2);
            }
        };
        
        float timeSec = (float)elapsedMs / 1000.0f;
        
        float r1 = radius * 0.4f * progress;
        if (r1 > 1.0f) DrawHex(r1, timeSec * 2.0f, style.strokeWidth, 1.0f, true);
        
        float r2 = radius * 0.7f * progress;
        if (r2 > 1.0f) DrawHex(r2, -timeSec * 0.5f, style.strokeWidth * 0.8f, 0.7f, false);
        
        float r3 = radius * 1.0f * progress;
        if (r3 > 1.0f) DrawHex(r3, timeSec * 1.0f + 0.5f, style.strokeWidth * 0.5f, 0.4f, false);
    }
};

// 4. Perspective 3D HUD Renderer
// 4. Hologram Renderer (Gyroscope style)
class HologramRenderer : public IRippleRenderer {
public:
    struct Vec3 { float x, y, z; };
    struct Particle { Vec3 pos; float speed; float life; };
    std::vector<Particle> particles_;

    // 3D Rotation Helpers
    Vec3 RotX(const Vec3& v, float angle) {
        float c = cos(angle), s = sin(angle);
        return { v.x, v.y * c - v.z * s, v.y * s + v.z * c };
    }
    Vec3 RotY(const Vec3& v, float angle) {
        float c = cos(angle), s = sin(angle);
        return { v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
    }
    Vec3 RotZ(const Vec3& v, float angle) {
        float c = cos(angle), s = sin(angle);
        return { v.x * c - v.y * s, v.x * s + v.y * c, v.z };
    }

    // Project 3D point to 2D
    Gdiplus::PointF Project(const Vec3& v, float cx, float cy) {
        // Camera Setup
        float dist = 500.0f; // Camera distance
        float scale = dist / (dist + v.z); 
        return Gdiplus::PointF(cx + v.x * scale, cy + v.y * scale);
    }

    void Start(const RippleStyle& style) override {
        particles_.clear();
        for(int i=0; i<40; ++i) {
            // Sphere cloud
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
        
        // Base Tilt for the whole structure to make it look 3D
        float baseTiltX = 20.0f * 3.14159f / 180.0f; 
        float baseTiltY = 10.0f * 3.14159f / 180.0f;

        Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        Gdiplus::Color glow = ToGdiPlus(style.glow);
        Gdiplus::Color fill = ToGdiPlus(style.fill);

        // --- Helper: Draw 3D Ring ---
        // rotAxis: 0=Flat(XY), 1=RotX, 2=RotY
        auto DrawRing3D = [&](float r, float spin, float width, Gdiplus::Color color, float startArc, float sweepArc, int axisMode, float axisTilt) {
             std::vector<Gdiplus::PointF> points;
             int segments = 60;
             // Only draw part of the arc?
             for (int i = 0; i <= segments; ++i) {
                 float pct = (float)i / segments;
                 float ang = startArc + sweepArc * pct;
                 
                 // Base ring geometry (in XY plane initially)
                 Vec3 v = { r * cos(ang), r * sin(ang), 0.0f };
                 
                 // Apply local ring rotation (Spinning)
                 // If axisMode=0 (Scanner), it just spins around Z
                 if (axisMode == 0) v = RotZ(v, spin);
                 else if (axisMode == 1) { v = RotZ(v, spin); v = RotX(v, axisTilt); } // Spin then tilt X
                 else if (axisMode == 2) { v = RotZ(v, spin); v = RotY(v, axisTilt); } // Spin then tilt Y
                 
                 // Apply Global Camera Tilt
                 v = RotX(v, baseTiltX);
                 v = RotY(v, baseTiltY);
                 
                 points.push_back(Project(v, cx, cy));
             }
             
             Gdiplus::Pen p(color, width);
             if (sweepArc < 6.28f) p.SetStartCap(Gdiplus::LineCapRound), p.SetEndCap(Gdiplus::LineCapRound);
             g.DrawLines(&p, points.data(), (INT)points.size());
        };

        // 1. Core Energy Orb
        {
            float coreR = style.strokeWidth * 6.0f;
            Gdiplus::GraphicsPath path;
            Vec3 center3D = {0,0,0};
            Gdiplus::PointF center = Project(center3D, cx, cy); // Should be cx,cy roughly
            
            path.AddEllipse(center.X - coreR, center.Y - coreR, coreR*2, coreR*2);
            Gdiplus::PathGradientBrush pgb(&path);
            
            // Bright White/Cyan Core
            pgb.SetCenterColor(Gdiplus::Color(255, 220, 240, 255)); 
            Gdiplus::Color surround[] = { Gdiplus::Color(0, stroke.GetR(), stroke.GetG(), stroke.GetB()) };
            int n = 1;
            pgb.SetSurroundColors(surround, &n);
            g.FillPath(&pgb, &path);
        }

        // 2. Inner Gyro Ring (Thin, fast spin)
        {
             BYTE a = ClampByte((int)(stroke.GetA() * 0.8f));
             Gdiplus::Color c(a, stroke.GetR(), stroke.GetG(), stroke.GetB());
             // Tilted 60 deg on X axis, spinning
             DrawRing3D(radius * 0.6f, timeSec * 1.5f, 2.0f, c, 0, 6.28f, 1, 60.0f * 3.14159f / 180.0f);
        }

        // 3. Middle Gyro Ring (Thin, cross spin)
        {
             BYTE a = ClampByte((int)(stroke.GetA() * 0.6f));
             Gdiplus::Color c(a, fill.GetR(), fill.GetG(), fill.GetB()); // Use Fill color (secondary)
             // Tilted 45 deg on Y axis, spinning opposite
             DrawRing3D(radius * 0.8f, -timeSec * 1.2f, 2.0f, c, 0, 6.28f, 2, 45.0f * 3.14159f / 180.0f);
        }

        // 4. Outer Scanner Ring (Thick, Loading Bar style)
        {
             // "Loading" arc: 75% complete
             float sweep = 6.28f * 0.75f * progress; 
             float spin = timeSec * 0.8f;
             
             BYTE a = ClampByte((int)(stroke.GetA()));
             Gdiplus::Color c(a, stroke.GetR(), stroke.GetG(), stroke.GetB());
             
             // Flat on Z plane (Scanner), no local axis tilt, just base tilt
             DrawRing3D(radius * 1.1f, spin, 5.0f * style.strokeWidth, c, 0.0f, sweep, 0, 0); // Thick
             
             // Counter-rotating thin ring segment
             DrawRing3D(radius * 1.15f, -spin * 1.5f, 1.0f, c, 0.0f, 2.0f, 0, 0); 
        }
        
        // 5. Particles (Electron Cloud)
        if(particles_.empty()) Start(style);
        
        Gdiplus::SolidBrush pb(Gdiplus::Color(200, stroke.GetR(), stroke.GetG(), stroke.GetB()));
        for(auto& p : particles_) {
            // Orbit visuals
            Vec3 pos = p.pos;
            // Rotate particle cloud over time
            pos = RotY(pos, timeSec * 0.5f);
            pos = RotX(pos, baseTiltX); // Apply camera tilt
            
            Gdiplus::PointF pt = Project(pos, cx, cy);
            float distScale = 500.0f / (500.0f + pos.z);
            float pSize = 3.0f * distScale;
            
            g.FillEllipse(&pb, pt.X - pSize/2, pt.Y - pSize/2, pSize, pSize);
        }
    }
};

// 5. Tech Ring Renderer (Previous "3D HUD" style with flat rings and particles)
// Note: Inserting INSIDE namespace by reopening or just ensuring it's technically inside if I replace correctly
// Wait, I should replace "} // namespace mousefx" with the class AND the closing brace.

    // 5. Tech Ring Renderer (Previous "3D HUD" style with flat rings and particles)
    class TechHudRenderer : public IRippleRenderer {
    public:
        struct Vec3 { float x, y, z; };
        struct Particle { Vec3 pos; float speed; float life; };
        std::vector<Particle> particles_;

        // Project 3D point to 2D
        Gdiplus::PointF Project(const Vec3& v, float cx, float cy, float tiltRad) {
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
                 for (int i = 0; i < count; ++i) {
                     float step = 2 * 3.14159f / count;
                     float startAng = i * step + spin;
                     float sweep = step * (1.0f - gapRatio);
                     
                     std::vector<Gdiplus::PointF> points;
                     int segs = 10;
                     for(int j=0; j<=segs; ++j) {
                         float ang = startAng + sweep * ((float)j/segs);
                         points.push_back(Project({r * cos(ang), 0.0f, r * sin(ang)}, cx, cy, tilt));
                     }
                     
                     BYTE a = ClampByte((int)(stroke.GetA() * (1.0f - progress * 0.5f))); 
                     Gdiplus::Pen p(Gdiplus::Color(a, stroke.GetR(), stroke.GetG(), stroke.GetB()), width);
                     g.DrawLines(&p, points.data(), (INT)points.size());
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
                pgb.SetCenterColor(Gdiplus::Color(255, 255, 255, 255));
                Gdiplus::Color surround[] = { Gdiplus::Color(0, stroke.GetR(), stroke.GetG(), stroke.GetB()) };
                int n = 1;
                pgb.SetSurroundColors(surround, &n);
                g.FillPath(&pgb, &path);
            }
        }
    };

} // namespace mousefx
