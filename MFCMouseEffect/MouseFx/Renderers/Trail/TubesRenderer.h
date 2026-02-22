#pragma once
#include "../../Interfaces/ITrailRenderer.h"
#include "MouseFx/Compute/EffectComputeExecutor.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/TrailColor.h"
#include <vector>
#include <cmath>
#include <gdiplus.h>

namespace mousefx {

class TubesRenderer : public ITrailRenderer {
public:
    struct TubeNode {
        float x = 0, y = 0;
    };

    struct TubeChain {
         std::vector<TubeNode> nodes;
         Gdiplus::Color color;
         // Physics params per chain for variety
         float lag = 0.5f; 
         float rotOffset = 0.0f;
    };

    TubesRenderer() {
        // Initialize 3 tubes based on reference colors: "#f967fb", "#53bc28", "#6958d5"
        // RGB conversion:
        // #f967fb -> 249, 103, 251
        // #53bc28 -> 83, 188, 40
        // #6958d5 -> 105, 88, 213

        chains_.resize(3);
        
        // Tube 1
        chains_[0].color = Gdiplus::Color(249, 103, 251); 
        chains_[0].nodes.resize(NUM_NODES);
        chains_[0].lag = 0.30f; // Slower follow (was 0.40f, original 0.25f)
        chains_[0].rotOffset = 0.0f;

        // Tube 2
        chains_[1].color = Gdiplus::Color(83, 188, 40);
        chains_[1].nodes.resize(NUM_NODES);
        chains_[1].lag = 0.40f; // Slower follow (was 0.50f, original 0.35f)
        chains_[1].rotOffset = 2.0f;

        // Tube 3
        chains_[2].color = Gdiplus::Color(105, 88, 213);
        chains_[2].nodes.resize(NUM_NODES);
        chains_[2].lag = 0.50f; // Slower follow (was 0.60f, original 0.45f)
        chains_[2].rotOffset = 4.0f;
    }

    void Render(Gdiplus::Graphics& g, const std::deque<TrailPoint>& points, int width, int height, Gdiplus::Color color, bool isChromatic) override {
        // 1. Determine Target
        POINT target = { 0, 0 };
        bool hasInput = !points.empty();
        
        if (hasInput) {
            target = points.back().pt;
            lastTarget_ = target;
            inited_ = true;
        } else {
            if (!inited_) return;
            target = lastTarget_;
        }

        // Global Alpha management for Disappearance
        uint64_t now = GetTickCount64();
        bool isIdle = !hasInput || (now - points.back().addedTime > 50);

        if (!isIdle) {
            // Mouse is actively moving: full opacity
            fadeAlpha_ = 255.0f;
        } else {
            // Mouse is idle for > 50ms: Check convergence & fade
            bool converged = true;
            for (const auto& chain : chains_) {
                if (!chain.nodes.empty()) {
                    const auto& head = chain.nodes[0];
                    const auto& tail = chain.nodes.back();
                    float dx = head.x - tail.x;
                    float dy = head.y - tail.y;
                    if (dx * dx + dy * dy > 100.0f) { // 10px threshold squared (was 25.0f/5px)
                        converged = false;
                        break;
                    }
                }
            }
            
            if (converged) {
                fadeAlpha_ -= 35.0f; // Faster fade (was 25.0f)
            } else {
                fadeAlpha_ -= 10.0f; // Gradual fade while converging
            }
            if (fadeAlpha_ < 0.0f) fadeAlpha_ = 0.0f;
        }
        
        if (fadeAlpha_ <= 0.0f) return; // Completely gone

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        // 2. Update physics by compute executor. Renderer only defines chain transition.
        const float targetX = (float)target.x;
        const float targetY = (float)target.y;
        ::mousefx::compute::ForEachIndex(
            (int)chains_.size(),
            ::mousefx::compute::ParallelProfile::AggressiveSmallBatch,
            [&](int idx) { UpdateChain(&chains_[(size_t)idx], targetX, targetY); });

        const float fadeScale = fadeAlpha_ / 255.0f;
        const float frameTime = (float)now * 0.005f;
        const float chromaBaseHue = std::fmod((float)now * 0.2f, 360.0f);

        // 3. Render
        // Draw from tail to head
        // The reference draws circles. Overlapping creates the tube look.
        
        // We render all chains. We can mix them or draw one after another.
        // Drawing one after another might look like layered ribbons. 
        // Mixing (Node 1 of A, Node 1 of B...) might look more intertwined? 
        // Let's draw completely separate passes for now to preserve color clarity.
        
        for (int c = 0; c < 3; ++c) {
            const auto& chain = chains_[c];
            // Gradient brush setup could be reused if optimized, but creating per node is safer for varying sizes/alphas.

            const int nodesCount = (int)chain.nodes.size();
            if (nodesCount <= 0) continue;
            const float invNodesCount = 1.0f / (float)nodesCount;
            const float chainPhase = (float)c * (2.0f * 3.14159f / 3.0f); // 0/120/240 degrees
            Gdiplus::Color chainBaseColor = chain.color;
            if (isChromatic) {
                const float hue = std::fmod(chromaBaseHue + (float)c * 30.0f, 360.0f);
                chainBaseColor = trail_color::HslToRgbColor(hue, 0.9f, 0.6f, 255);
            }

            for (int i = nodesCount - 1; i >= 0; --i) {
                const auto& node = chain.nodes[i];
                
                // Size tapers from Head(large) to Tail(small) or vice versa?
                // Visual check: usually Head is lead, fairly large. Tail fades out.
                // Let's try: Head = 20px, Tail = 2px.
                
                const float ratio = 1.0f - (float)i * invNodesCount; // 1.0 at head, ~0.0 at tail
                float radius = 2.0f + 7.0f * ratio; // Reduced from 12.0f
                
                // Shrink when fading
                if (fadeAlpha_ < 255.0f) {
                    radius *= fadeScale;
                } 
                
                const ScreenPoint nodePt{
                    static_cast<int32_t>(std::lround(node.x)),
                    static_cast<int32_t>(std::lround(node.y))};
                const ScreenPoint localPt = ScreenToOverlayPoint(nodePt);
                float renderX = (float)localPt.x;
                float renderY = (float)localPt.y;
                
                // --- Helix/Weave Offset ---
                // To prevent them from merging into a single line, we add a perpendicular or radial offset.
                // We simulate a spiral around the central path.
                
                // Offset based on node index (to create the spiral wave along the tail)
                const float nodePhase = (float)i * 0.3f; 
                
                // Amplitude tapers off at the tail? Or fully thick?
                // Visual choice: Tapering amplitude makes it look like a drill/tornado.
                // Constant amplitude makes it look like a cable. 
                // Let's use constant amplitude relative to radius, but maybe scale by ratio to keep head tight?
                // Actually, loose tail looks better.
                float amplitude = 8.0f; 
                
                // If disappearing, reduce amplitude to 0 to converge cleanly
                if (fadeAlpha_ < 255.0f) {
                    amplitude *= fadeScale;
                }

                const float oscX = std::cos(frameTime + nodePhase + chainPhase) * amplitude;
                const float oscY = std::sin(frameTime + nodePhase + chainPhase) * amplitude;
                
                renderX += oscX;
                renderY += oscY;
                // ---------------------------

                Gdiplus::GraphicsPath path;
                path.AddEllipse(renderX - radius, renderY - radius, radius * 2, radius * 2);
    
                Gdiplus::PathGradientBrush pthGrBrush(&path);
                
                // Color calc
                // Center is whitish, surround is the chain color
                const Gdiplus::Color base = chainBaseColor;

                int alpha = (int)(255 * ratio); // Tail fades out alpha too
                alpha = (int)(alpha * fadeScale); // Apply global fade
                
                // Adjust base color with alpha
                Gdiplus::Color surroundC(0, base.GetR(), base.GetG(), base.GetB()); // 0 alpha at edge

                // Ideally center is solid white at head?
                // Reference has a very "glowing" look. 
                // Let's try: Center = BaseColor mixed with White, high alpha. Surround = BaseColor, 0 alpha.
                
                // Center:
                Gdiplus::Color cColor((BYTE)alpha, 
                    (BYTE)std::min(255, base.GetR() + 100),
                    (BYTE)std::min(255, base.GetG() + 100),
                    (BYTE)std::min(255, base.GetB() + 100)
                );
                
                int count = 1;
                pthGrBrush.SetCenterColor(cColor);
                pthGrBrush.SetSurroundColors(&surroundC, &count);
                
                // Determine focus scale (center point size)
                // Default is 0,0 (center point). 
                
                g.FillEllipse(&pthGrBrush, renderX - radius, renderY - radius, radius * 2, radius * 2);
            }
        }
    }

private:
    static void UpdateChain(TubeChain* chain, float targetX, float targetY) {
        if (!chain || chain->nodes.empty()) return;

        // Update head.
        auto& head = chain->nodes[0];
        const float dx = targetX - head.x;
        const float dy = targetY - head.y;
        head.x += dx * chain->lag;
        head.y += dy * chain->lag;

        // Propagate to tail with minimum segment distance to avoid clumping.
        for (size_t i = 1; i < chain->nodes.size(); ++i) {
            auto& curr = chain->nodes[i];
            auto& prev = chain->nodes[i - 1];

            const float ddx = prev.x - curr.x;
            const float ddy = prev.y - curr.y;
            curr.x += ddx * chain->lag;
            curr.y += ddy * chain->lag;

            constexpr float kMinSegmentDist = 3.5f;
            const float dist = std::sqrt(ddx * ddx + ddy * ddy);
            if (dist < kMinSegmentDist && dist > 0.01f) {
                const float nx = ddx / dist;
                const float ny = ddy / dist;
                curr.x = prev.x - nx * kMinSegmentDist;
                curr.y = prev.y - ny * kMinSegmentDist;
            }
        }
    }

    static constexpr size_t NUM_NODES = 30; // Length of tail
    std::vector<TubeChain> chains_;
    POINT lastTarget_ = { 0, 0 };

    bool inited_ = false;
    float fadeAlpha_ = 255.0f;
};

} // namespace mousefx
