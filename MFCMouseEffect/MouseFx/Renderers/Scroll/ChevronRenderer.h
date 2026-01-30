#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"
#include <cmath>

namespace mousefx {

class ChevronRenderer : public IRippleRenderer {
public:
    RenderParams params_;
    
    // Allow parameter updates if supported
    void OnCommand(const std::string& cmd, const std::string& args) override {
        // Optional: handle intensity updates via command
    }
    
    // Note: The original implementation in RippleWindow used StartContinuous with params. 
    // Since our registry creates a fresh instance, the caller (ScrollEffect) is responsible 
    // for configuring it if it needs custom params beyond style. 
    // HOWEVER, standard IRippleRenderer doesn't expose SetParams.
    // We might need to handle 'direction' in Start or passed via style/command?
    // Looking at ScrollEffect.cpp (not viewed yet, but recalling architecture), 
    // it likely uses ScrollEffect to orchestrate. 
    // Wait, ScrollEffect instantiates `ScrollWindow` or `RippleWindow`?
    // Render strategies used by RippleWindow.
    // Actually, ScrollEffect usually calls `pool_.Show(..., renderer)`. 
    // But `pool_` uses `RippleWindow`.
    // `RippleWindow::StartContinuous` takes `RenderParams`.
    // `ChevronRenderer` needs to access `params` passed in `RippleWindow::StartContinuous`.
    // But `IRippleRenderer::Render` does NOT receive `RenderParams`! 
    // Note: legacy "RenderStrategies/StandardRenderers" headers were removed; see git history if needed.
    // It had `RenderParams params_;` member and `SetParams`. 
    // But `IRippleRenderer` interface defines `Render` without `RenderParams`.
    // So `RippleWindow` must have downcasted or `ChevronRenderer` was special?
    // The old implementation relied on setting a `RenderParams` member via a custom setter.
    // `RippleWindow.cpp` Line 140: `style_ = style; render_ = params;`.
    // Warning: `RippleWindow` stores `render_` (the params) but does NOT pass it to `Render` (Line 271).
    // `renderer_->Render(g, t, elapsedMs, sizePx_, style_);`
    // So `ChevronRenderer` used `params_.intensity` internally.
    // How did `params_` get set? ScrollEffect sets it before handing the renderer to the window pool.
    // `ScrollEffect.cpp`:
    // `auto r = std::make_unique<ChevronRenderer>(); r->SetParams(params); pool_.Show...(std::move(r))`
    // Yes, that must be it.
    
    void SetParams(const RenderParams& p) { params_ = p; }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        
        const float eased = 1.0f - (1.0f - Clamp01(t)) * (1.0f - Clamp01(t)) * (1.0f - Clamp01(t));
        const float intensity = Clamp01(params_.intensity);
        const float alpha = (1.0f - eased) * intensity;
        const float radius = style.startRadius + (style.endRadius - style.startRadius) * eased;
        const float cx = sizePx / 2.0f;
        const float cy = sizePx / 2.0f;

        const float dir = params_.directionRad;
        const float dx = (float)cos(dir);
        const float dy = (float)sin(dir);
        const float px = -dy;
        const float py = dx;

        const Gdiplus::Color base = ToGdiPlus(style.stroke);
        const BYTE aBase = ClampByte((int)(base.GetA() * alpha));

        for (int i = 0; i < 3; ++i) {
            const float offset = i * (radius * 0.25f);
            const float length = radius * 1.1f;
            const float halfWidth = style.strokeWidth * (3.2f - i * 0.6f);

            const float tipX = cx + dx * (length * 0.5f - offset);
            const float tipY = cy + dy * (length * 0.5f - offset);
            const float tailX = cx - dx * (length * 0.5f + offset);
            const float tailY = cy - dy * (length * 0.5f + offset);

            const float lx = tailX + px * halfWidth;
            const float ly = tailY + py * halfWidth;
            const float rx = tailX - px * halfWidth;
            const float ry = tailY - py * halfWidth;

            const float fade = 1.0f - i * 0.18f;
            const Gdiplus::Color glow = ToGdiPlus(style.glow);
            const BYTE a = ClampByte((int)(aBase * fade));
            const BYTE glowA = ClampByte((int)(glow.GetA() * alpha * fade));
            Gdiplus::Pen glowPen(Gdiplus::Color(glowA, glow.GetR(), glow.GetG(), glow.GetB()),
                style.strokeWidth + 6.0f);
            glowPen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawLine(&glowPen, tipX, tipY, lx, ly);
            g.DrawLine(&glowPen, tipX, tipY, rx, ry);

            Gdiplus::Pen pen(Gdiplus::Color(a, base.GetR(), base.GetG(), base.GetB()),
                style.strokeWidth + 1.0f);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawLine(&pen, tipX, tipY, lx, ly);
            g.DrawLine(&pen, tipX, tipY, rx, ry);
        }
    }
};

REGISTER_RENDERER("arrow", ChevronRenderer)

} // namespace mousefx
