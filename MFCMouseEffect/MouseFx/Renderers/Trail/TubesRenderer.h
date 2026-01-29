#pragma once
#include "../../ITrailRenderer.h"
#include <vector>
#include <cmath>
#include <gdiplus.h>

namespace mousefx {

class TubesRenderer : public ITrailRenderer {
public:
    TubesRenderer() {
        // Initialize physics nodes
        // Start with some nodes at 0,0
        nodes_.resize(NUM_NODES);
        for (auto& n : nodes_) {
            n.x = 0; n.y = 0;
            n.vx = 0; n.vy = 0;
        }
    }

    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        // Physics update loop
        // We use the latest point from 'points' as the target for the head node.
        // If points is empty, we keep the last known target (or 0,0 if never set).
        
        POINT target = { 0, 0 };
        bool hasTarget = false;

        if (!points.empty()) {
            target = points.back().pt;
            lastTarget_ = target;
            hasTarget = true;
        } else {
            // Use last known position if no new input
            target = lastTarget_;
            // Ideally we check if we ever had a target
            if (target.x == 0 && target.y == 0) return; 
        }

        // --- Physics Simulation ---
        // Spring-Damper parameters
        // Creating a "chain" where Node 0 follows Target, Node 1 follows Node 0, etc.
        
        float spring = 0.4f;  // Stiffness (how fast it moves to target)
        float friction = 0.25f; // Damping (how fast velocity decays)
        // Gravity? No.

        // Update Head (Node 0)
        {
            auto& head = nodes_[0];
            float dx = (float)target.x - head.x;
            float dy = (float)target.y - head.y;
            
            head.vx += dx * spring;
            head.vy += dy * spring;
            head.vx *= friction;
            head.vy *= friction;
            head.x += head.vx;
            head.y += head.vy;
        }

        // Update Tail (Node i follows Node i-1)
        float restLength = 10.0f; // Maintain distance between segments
        
        for (size_t i = 1; i < nodes_.size(); ++i) {
            auto& node = nodes_[i];
            auto& prev = nodes_[i-1];

            float dx = prev.x - node.x;
            float dy = prev.y - node.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            // Normalize direction
            float nx = 0.0f, ny = 0.0f;
            if (dist > 0.001f) {
                nx = dx / dist;
                ny = dy / dist;
            }

            // Spring force based on displacement from rest length
            // We want them to bunch up a bit but not collapse.
            // Actually, for "Tube" effect, they often collapse but have "Orbital" noise.
            // Let's use a soft rest length.
            float safeDist = std::max(dist, 1.0f);
            float force = (safeDist - 5.0f) * spring; // 5.0f is minimal separation

            node.vx += nx * force;
            node.vy += ny * force;
            
            // Add some "noise" or "orbit" if idle?
            // If very slow, add tangential force
            if (dist < 20.0f) {
                // Tangential
               // node.vx += -ny * 0.5f;
               // node.vy += nx * 0.5f;
            }

            node.vx *= friction;
            node.vy *= friction;
            node.x += node.vx;
            node.y += node.vy;
        }

        // --- Rendering ---
        int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        // Draw from tail to head or head to tail?
        // Head is newest (top). Tail is oldest (bottom).
        // Draw tail first so head is on top.

        for (size_t i = nodes_.size(); i-- > 0; ) {
            const auto& node = nodes_[i];
            
            // Radius can pulsate or vary by index
            // Head (0) is largest? Or Tail is largest? Tubes usually taper?
            // Let's try Head = Large, Tail = Small
            float ratio = 1.0f - (float)i / (float)nodes_.size(); // 1.0 at head, 0.0 at tail
            float radius = 5.0f + 25.0f * ratio; // 5 to 30 radius

            // Color
            Gdiplus::Color c = color;
            if (isChromatic) {
                // Time based hue shift?
                // Or index based?
                // Let's use a static hue shift based on index for the "tube" look
                // But we need 'now' for animation.
                // ITrailRenderer doesn't pass 'now', but we can get it.
                uint64_t now = GetTickCount64();
                float hue = std::fmod((float)now * 0.2f + i * 15.0f, 360.0f);
                c = HslToRgb(hue, 0.9f, 0.6f, 255);
            }

            // PathGradientBrush for "glow" ball look
            Gdiplus::GraphicsPath path;
            
            // Add "3D" Spiral Offset
            // When grouped, this makes them look like a ring/cluster.
            // When trailing, it makes the tail spiral.
            uint64_t now = GetTickCount64();
            float timeOffset = (float)now * 0.002f; // Rotation speed
            float indexOffset = (float)i * 0.5f;    // Spiral tightness
            float orbitRadius = 15.0f * (1.0f - ratio); // Tail orbits wider? Or head? 
            // actually if we want a "cluster" at mouse, maybe uniform orbit?
            orbitRadius = 12.0f; 

            float offX = std::cos(timeOffset + indexOffset) * orbitRadius;
            float offY = std::sin(timeOffset + indexOffset) * orbitRadius;

            float renderX = node.x - x_offset + offX;
            float renderY = node.y - y_offset + offY;
            
            path.AddEllipse(renderX - radius, renderY - radius, radius * 2, radius * 2);

            Gdiplus::PathGradientBrush pthGrBrush(&path);
            
            // Center color (white/bright) -> Surround color (theme color transparent)
            pts_colors_[0] = Gdiplus::Color(255, 255, 255, 255); // Center white
            
            int alpha = 150 + (int)(100 * ratio); // Head more opaque
            if (alpha > 255) alpha = 255;
            
            Gdiplus::Color surroundColor(alpha, c.GetR(), c.GetG(), c.GetB());
            int count = 1;
            pthGrBrush.SetCenterColor(surroundColor); // Actually center should be bright? 
            // Let's try: Center = Bright Theme Color, Surround = Transparent Theme Color
            
            // Refined look:
            // Center: White with some alpha
            // Surround: Theme color with 0 alpha (soft edge)
            
            Gdiplus::Color centerC(255, std::min(255, (int)(c.GetR() + 50)), std::min(255, (int)(c.GetG() + 50)), std::min(255, (int)(c.GetB() + 50)));
            Gdiplus::Color edgeC(0, c.GetR(), c.GetG(), c.GetB());
            
            pthGrBrush.SetCenterColor(centerC);
            pthGrBrush.SetSurroundColors(&edgeC, &count);
            
            // Focus scale to make the white center smaller
            // pthGrBrush.SetFocusScales(0.2f, 0.2f); 

            g.FillEllipse(&pthGrBrush, renderX - radius, renderY - radius, radius * 2, radius * 2);
        }
    }

private:
    struct Node {
        float x, y;
        float vx, vy;
    };

    static constexpr size_t NUM_NODES = 25;
    std::vector<Node> nodes_;
    POINT lastTarget_ = { 0, 0 };
    Gdiplus::Color pts_colors_[1]; // Logic helper

    // Helper for HSL (Duplicated from RenderUtils/TrailWindow - should unify later)
    Gdiplus::Color HslToRgb(float h, float s, float l, int alpha) {
        auto hue2rgb = [](float p, float q, float t) {
            if (t < 0.0f) t += 1.0f;
            if (t > 1.0f) t -= 1.0f;
            if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
            if (t < 1.0f / 2.0f) return q;
            if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        };
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        float tr = h / 360.0f + 1.0f / 3.0f;
        float tg = h / 360.0f;
        float tb = h / 360.0f - 1.0f / 3.0f;
        return Gdiplus::Color((BYTE)alpha, (BYTE)(hue2rgb(p, q, tr) * 255), (BYTE)(hue2rgb(p, q, tg) * 255), (BYTE)(hue2rgb(p, q, tb) * 255));
    }
};

} // namespace mousefx
