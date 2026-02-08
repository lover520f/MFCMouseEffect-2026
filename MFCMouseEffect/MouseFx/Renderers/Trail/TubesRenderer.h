#pragma once
#include "../../Interfaces/ITrailRenderer.h"
#include "MouseFx/Core/OverlayCoordSpace.h"
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

        // 2. Update Physics for each chain
        for (int c = 0; c < 3; ++c) {
            auto& chain = chains_[c];
            
            // Head follows mouse
            float targetX = (float)target.x;
            float targetY = (float)target.y;

            // Simple exponential smoothing (Lerp) for the head
            // The reference implementation might use a more complex array shifting, 
            // but Lerp gives a good "damped" feel.
            
            // To emulate the "Tubes" library:
            // It likely propagates positions down the chain.
            
            // Update Head
            auto& head = chain.nodes[0];
            float dx = targetX - head.x;
            float dy = targetY - head.y;
            
            // "Lag" factor determines speed. detailed logic:
            // if we just lerp, it never overshoots.
            // Reference seems to be simple follow.
            head.x += dx * chain.lag;
            head.y += dy * chain.lag;

            // Propagate to rest of chain
            for (size_t i = 1; i < chain.nodes.size(); ++i) {
                auto& curr = chain.nodes[i];
                auto& prev = chain.nodes[i-1];
                
                float ddx = prev.x - curr.x;
                float ddy = prev.y - curr.y;
                
                // Follow previous node
                curr.x += ddx * chain.lag;
                curr.y += ddy * chain.lag;
                
                // --- Minimum distance constraint to prevent clumping ---
                // Ensure nodes are at least MIN_SEGMENT_DIST apart
                constexpr float MIN_SEGMENT_DIST = 3.5f;
                float dist = std::sqrt(ddx * ddx + ddy * ddy);
                if (dist < MIN_SEGMENT_DIST && dist > 0.01f) {
                    // Push the current node away from the previous node
                    float nx = ddx / dist;
                    float ny = ddy / dist;
                    curr.x = prev.x - nx * MIN_SEGMENT_DIST;
                    curr.y = prev.y - ny * MIN_SEGMENT_DIST;
                }
            }
        }

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
            
            int nodesCount = (int)chain.nodes.size();
            for (int i = nodesCount - 1; i >= 0; --i) {
                const auto& node = chain.nodes[i];
                
                // Size tapers from Head(large) to Tail(small) or vice versa?
                // Visual check: usually Head is lead, fairly large. Tail fades out.
                // Let's try: Head = 20px, Tail = 2px.
                
                float ratio = 1.0f - (float)i / (float)nodesCount; // 1.0 at head, ~0.0 at tail
                float radius = 2.0f + 7.0f * ratio; // Reduced from 12.0f
                
                // Shrink when fading
                if (fadeAlpha_ < 255.0f) {
                    radius *= (fadeAlpha_ / 255.0f);
                } 
                
                const POINT nodePt{(LONG)std::lround(node.x), (LONG)std::lround(node.y)};
                const POINT localPt = ScreenToOverlayPoint(nodePt);
                float renderX = (float)localPt.x;
                float renderY = (float)localPt.y;
                
                // --- Helix/Weave Offset ---
                // To prevent them from merging into a single line, we add a perpendicular or radial offset.
                // We simulate a spiral around the central path.
                
                uint64_t now = GetTickCount64();
                float time = (float)now * 0.005f; // Animation speed
                
                // Offset based on chain ID (angles 0, 120, 240 degrees)
                float chainPhase = c * (2.0f * 3.14159f / 3.0f); 
                
                // Offset based on node index (to create the spiral wave along the tail)
                float nodePhase = i * 0.3f; 
                
                // Amplitude tapers off at the tail? Or fully thick?
                // Visual choice: Tapering amplitude makes it look like a drill/tornado.
                // Constant amplitude makes it look like a cable. 
                // Let's use constant amplitude relative to radius, but maybe scale by ratio to keep head tight?
                // Actually, loose tail looks better.
                float amplitude = 8.0f; 
                
                // If disappearing, reduce amplitude to 0 to converge cleanly
                if (fadeAlpha_ < 255.0f) {
                    amplitude *= (fadeAlpha_ / 255.0f);
                }

                float oscX = std::cos(time + nodePhase + chainPhase) * amplitude;
                float oscY = std::sin(time + nodePhase + chainPhase) * amplitude;
                
                renderX += oscX;
                renderY += oscY;
                // ---------------------------

                Gdiplus::GraphicsPath path;
                path.AddEllipse(renderX - radius, renderY - radius, radius * 2, radius * 2);
    
                Gdiplus::PathGradientBrush pthGrBrush(&path);
                
                // Color calc
                // Center is whitish, surround is the chain color
                Gdiplus::Color base = chain.color;
                
                if (isChromatic) {
                   // Chromatic: Cycle hue based on time and chain index
                   // Time factor: slow cycle (0.1)
                   // Chain offset: separate them (30 degrees)
                   // Node offset: slight gradient along tail? (0.5 * i)
                   
                   uint64_t now = GetTickCount64();
                   float hue = std::fmod((float)now * 0.2f + c * 30.0f, 360.0f);
                   
                   // Convert to RGB
                   base = trail_color::HslToRgbColor(hue, 0.9f, 0.6f, 255);
                }

                int alpha = (int)(255 * ratio); // Tail fades out alpha too
                alpha = (int)(alpha * (fadeAlpha_ / 255.0f)); // Apply global fade
                
                // Adjust base color with alpha
                Gdiplus::Color surroundC(0, base.GetR(), base.GetG(), base.GetB()); // 0 alpha at edge
                Gdiplus::Color centerC(alpha, 255, 255, 255); // White center, varying alpha

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
    static constexpr size_t NUM_NODES = 30; // Length of tail
    std::vector<TubeChain> chains_;
    POINT lastTarget_ = { 0, 0 };

    bool inited_ = false;
    float fadeAlpha_ = 255.0f;
};

} // namespace mousefx
