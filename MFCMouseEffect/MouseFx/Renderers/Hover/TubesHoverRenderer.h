#pragma once
#include "../../Interfaces/IRippleRenderer.h"
#include "MouseFx/Compute/EffectComputeExecutor.h"
#include "MouseFx/Utils/TimeUtils.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <gdiplus.h>

namespace mousefx {

namespace { // Helper for HSL (local scope)
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

class TubesHoverRenderer : public IRippleRenderer {
public:
    struct HoverNode {
        float x = 0, y = 0;
        float baseOffX = 0, baseOffY = 0; // Base offset from center
        float phase = 0;
    };

    struct HoverChain {
         std::vector<HoverNode> nodes;
         Gdiplus::Color color;
         float speed = 0.05f;
         float radiusScale = 1.0f;
    };

    TubesHoverRenderer(bool isChromatic = false) : isChromatic_(isChromatic) {
        // Init 3 chains matching the trail effect colors
        chains_.resize(3);
        frameScratch_.resize(3);
        
        chains_[0].color = Gdiplus::Color(249, 103, 251); 
        chains_[0].nodes.resize(NUM_NODES);
        chains_[0].speed = 0.002f;
        chains_[0].radiusScale = 0.8f;

        chains_[1].color = Gdiplus::Color(83, 188, 40);
        chains_[1].nodes.resize(NUM_NODES);
        chains_[1].speed = 0.003f;
        chains_[1].radiusScale = 1.0f;

        chains_[2].color = Gdiplus::Color(105, 88, 213);
        chains_[2].nodes.resize(NUM_NODES);
        chains_[2].speed = 0.0015f;
        chains_[2].radiusScale = 1.2f;

        // Initialize spiral / circle layout for suspension
        for (int c = 0; c < 3; ++c) {
            for (int i = 0; i < NUM_NODES; ++i) {
                // Nodes arranged in a circle or spiral
                float angle = (float)i / NUM_NODES * 6.28f * 2.0f; // 2 loops
                float r = (10.0f + i * 2.0f) * chains_[c].radiusScale;
                chains_[c].nodes[i].baseOffX = std::cos(angle) * r;
                chains_[c].nodes[i].baseOffY = std::sin(angle) * r;
                chains_[c].nodes[i].phase = angle;
            }
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        // "Suspension" effect:
        // The tubes gently rotate or undulate around the center of the window.
        // sizePx is the window size (width/height). 
        
        float cx = sizePx / 2.0f;
        float cy = sizePx / 2.0f;
        const uint64_t nowTick = NowMs();
        
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        for (int c = 0; c < 3; ++c) {
            auto& chain = chains_[c];
            // Use global time to avoid resets when overlay sessions are restarted
            const float time = (float)nowTick * chain.speed;
            Gdiplus::Color chainBase = chain.color;
            if (isChromatic_) {
                const float hue = std::fmod((float)nowTick * 0.2f + (float)c * 30.0f, 360.0f);
                chainBase = HslToRgb(hue, 0.9f, 0.6f, 255);
            }
            const float invCount = chain.nodes.empty() ? 0.0f : (1.0f / (float)chain.nodes.size());
            std::vector<NodeFrame>& frames = frameScratch_[(size_t)c];
            ::mousefx::compute::BuildArrayInto<NodeFrame>(
                frames,
                (int)chain.nodes.size(),
                ::mousefx::compute::ParallelProfile::Throughput,
                [&](int idx) {
                    NodeFrame frame{};
                    const auto& node = chain.nodes[(size_t)idx];

                    float rot = time + node.phase;
                    float curR = std::sqrt(node.baseOffX * node.baseOffX + node.baseOffY * node.baseOffY);
                    curR += std::sin(time * 2.0f + (float)idx * 0.1f) * 5.0f;

                    frame.x = cx + std::cos(rot) * curR;
                    frame.y = cy + std::sin(rot) * curR;

                    const float ratio = 1.0f - (float)idx * invCount;
                    frame.radius = 3.0f + 10.0f * ratio;

                    frame.base = chainBase;
                    return frame;
                });

            for (const auto& frame : frames) {
                const float finalX = frame.x;
                const float finalY = frame.y;
                const float radius = frame.radius;

                Gdiplus::GraphicsPath path;
                path.AddEllipse(finalX - radius, finalY - radius, radius * 2, radius * 2);
    
                Gdiplus::PathGradientBrush pthGrBrush(&path);
                
                const Gdiplus::Color base = frame.base;
                
                // Alpha fade? Suspension is usually fully visible but ghosty?
                // Use style.alpha multiplier if needed, but fixed 200 is fine for this specific effect.
                int alpha = 200; 

                Gdiplus::Color surroundC(0, base.GetR(), base.GetG(), base.GetB());
                
                // Slight mix with base color for center
                 Gdiplus::Color mixC(alpha, 
                    (BYTE)std::min(255, base.GetR() + 150),
                    (BYTE)std::min(255, base.GetG() + 150),
                    (BYTE)std::min(255, base.GetB() + 150)
                );

                int count = 1;
                pthGrBrush.SetCenterColor(mixC);
                pthGrBrush.SetSurroundColors(&surroundC, &count);
                
                g.FillEllipse(&pthGrBrush, finalX - radius, finalY - radius, radius * 2, radius * 2);
            }
        }
    }

private:
    struct NodeFrame {
        float x = 0.0f;
        float y = 0.0f;
        float radius = 0.0f;
        Gdiplus::Color base{};
    };

    static constexpr int NUM_NODES = 20;

    std::vector<HoverChain> chains_;
    std::vector<std::vector<NodeFrame>> frameScratch_;
    bool isChromatic_ = false;
};

} // namespace mousefx
