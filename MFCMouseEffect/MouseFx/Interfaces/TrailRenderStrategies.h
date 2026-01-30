#pragma once
#include "ITrailRenderer.h"
#include <vector>
#include <cmath>

namespace mousefx {

namespace trail_utils {
    inline Gdiplus::Color HslToRgb(float h, float s, float l, int alpha) {
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
}

// 1. Standard Line Renderer (Legacy)
class LineTrailRenderer : public ITrailRenderer {
public:
    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        if (points.size() < 2) return;
        
        uint64_t now = GetTickCount64();
        int durationMs = 300; // Default duration for Line

        int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        for (size_t i = 0; i < points.size() - 1; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i+1];
            
            uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs);
            if (life < 0) life = 0;
            
            int alpha = (int)(life * 255);
            Gdiplus::Color c(alpha, color.GetR(), color.GetG(), color.GetB());
            
            if (isChromatic) {
                 float hue = std::fmod((float)now * 0.1f + i * 10.0f, 360.0f);
                 c = trail_utils::HslToRgb(hue, 0.8f, 0.6f, alpha);
            }

            Gdiplus::Pen p(c, 4.0f);
            p.SetStartCap(Gdiplus::LineCapRound);
            p.SetEndCap(Gdiplus::LineCapRound);

            g.DrawLine(&p, (int)p1.pt.x - x_offset, (int)p1.pt.y - y_offset, (int)p2.pt.x - x_offset, (int)p2.pt.y - y_offset);
        }
    }
};

// 2. Neon Streamer Renderer (Smooth Tapered Ribbon)
class StreamerTrailRenderer : public ITrailRenderer {
public:
    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        if (points.size() < 4) return; // Need curve points

        uint64_t now = GetTickCount64();
        int durationMs = 500; // Longer duration for streamer

        int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);

        // Draw segments with varying width
        // A proper ribbon requires constructing a polygon strip, but we can fake it with thick lines that overlap well
        
        for (size_t i = 0; i < points.size() - 1; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i+1];

            uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs);
            if (life <= 0) continue;

            // Width tapers from Head (new) to Tail (old)
            // p1 is older (closer to 0 index?), wait points are pushed back?
            // points_.push_back(tp) means newer points are at the END.
            // So Index 0 is oldest. Index size-1 is newest.
            
            // Correction: "life" calculation above assumes p1 is old?
            // if age is large, life is small. So older points have less life. Correct.
            
            float width = 2.0f + 16.0f * life; // Thick head, thin tail
            int alpha = (int)(life * 200);     // Slightly transparent

            Gdiplus::Color c(alpha, color.GetR(), color.GetG(), color.GetB());
            if (isChromatic) {
                 float hue = std::fmod((float)now * 0.2f + i * 5.0f, 360.0f);
                 c = trail_utils::HslToRgb(hue, 0.9f, 0.6f, alpha);
            }

            Gdiplus::Pen p(c, width);
            p.SetStartCap(Gdiplus::LineCapRound);
            p.SetEndCap(Gdiplus::LineCapRound);
            
            // Smooth joint
            p.SetLineJoin(Gdiplus::LineJoinRound);

            g.DrawLine(&p, (int)p1.pt.x - x_offset, (int)p1.pt.y - y_offset, (int)p2.pt.x - x_offset, (int)p2.pt.y - y_offset);
            
            // Glow pass (add a wider, more transparent line behind or on top?)
            // Doing it per segment is expensive/overlapping.
            // But for simple streamer it works.
        }
    }
};

// 3. Electric Arc Renderer
class ElectricTrailRenderer : public ITrailRenderer {
public:
    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        if (points.size() < 2) return;

        uint64_t now = GetTickCount64();
        int durationMs = 400;

        int x_offset = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y_offset = GetSystemMetrics(SM_YVIRTUALSCREEN);
        
        srand((unsigned int)now); // Jitter changes every frame

        for (size_t i = 0; i < points.size() - 1; ++i) {
            const auto& p1 = points[i];
            const auto& p2 = points[i+1];

            uint64_t age = now - p1.addedTime;
            float life = 1.0f - ((float)age / (float)durationMs);
            if (life <= 0) continue;

            int alpha = (int)(life * 255);
            Gdiplus::Color c(alpha, color.GetR(), color.GetG(), color.GetB());
            if (isChromatic) {
                 float hue = std::fmod((float)now * 0.5f + i * 15.0f, 360.0f);
                 c = trail_utils::HslToRgb(hue, 1.0f, 0.7f, alpha);
            }

            Gdiplus::Pen p(c, (float)(2.0f * life));
            
            float x1 = (float)p1.pt.x - x_offset;
            float y1 = (float)p1.pt.y - y_offset;
            float x2 = (float)p2.pt.x - x_offset;
            float y2 = (float)p2.pt.y - y_offset;

            // Divide segment into sub-segments for jitter
            int segs = 3;
            float dx = (x2 - x1) / segs;
            float dy = (y2 - y1) / segs;
            
            float curX = x1;
            float curY = y1;

            for(int j=0; j<segs; ++j) {
                float nextX = (j == segs - 1) ? x2 : (x1 + dx * (j+1));
                float nextY = (j == segs - 1) ? y2 : (y1 + dy * (j+1));
                
                // Jitter
                if (j < segs - 1) {
                    float jitter = 5.0f * life;
                    nextX += ((float)(rand()%100)/50.0f - 1.0f) * jitter;
                    nextY += ((float)(rand()%100)/50.0f - 1.0f) * jitter;
                }
                
                g.DrawLine(&p, curX, curY, nextX, nextY);
                curX = nextX;
                curY = nextY;
            }
        }
    }
};

} // namespace mousefx
