#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"

#include <vector>
#include <cmath>
#include <cstdio>
#include <string>

namespace mousefx {

class FluxFieldHudCpuRenderer final : public IRippleRenderer {
public:
    struct OrbitParticle {
        float radius = 0.0f;
        float angle = 0.0f;
        float speed = 0.0f;
        float pulse = 0.0f;
    };

    void Start(const RippleStyle& style) override {
        currentHoldMs_ = 0;
        thresholdMs_ = style.durationMs;
        holdBiasMs_ = 0;
        holdBiasValid_ = false;
        particles_.clear();
        const float base = (style.endRadius > 0.0f) ? style.endRadius : 96.0f;
        for (int i = 0; i < 84; ++i) {
            OrbitParticle p{};
            p.radius = base * (0.18f + 0.78f * ((float)((i * 37) % 100) / 100.0f));
            p.angle = ((float)(i * 29) * 3.1415926f / 180.0f);
            p.speed = 0.45f + 0.75f * ((float)((i * 17) % 100) / 100.0f);
            p.pulse = (float)((i * 11) % 100) / 100.0f;
            particles_.push_back(p);
        }
    }

    void OnCommand(const std::string& cmd, const std::string& args) override {
        if (cmd == "hold_ms") {
            uint32_t ms = 0;
            if (sscanf_s(args.c_str(), "%u", &ms) == 1) currentHoldMs_ = ms;
            return;
        }
        if (cmd == "threshold_ms") {
            uint32_t ms = 0;
            if (sscanf_s(args.c_str(), "%u", &ms) == 1) thresholdMs_ = ms;
            return;
        }
        if (cmd == "hold_state") {
            uint32_t ms = 0;
            int x = 0;
            int y = 0;
            if (sscanf_s(args.c_str(), "%u,%d,%d", &ms, &x, &y) >= 1) {
                currentHoldMs_ = ms;
            }
            return;
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;
        if (particles_.empty()) Start(style);

        const float cx = sizePx * 0.5f;
        const float cy = sizePx * 0.5f;
        const float progress = ComputeProgress(t, elapsedMs, style.durationMs);
        const float timeSec = (float)elapsedMs / 1000.0f;
        const float radius = style.startRadius + (style.endRadius - style.startRadius) * progress;
        const Gdiplus::Color stroke = ToGdiPlus(style.stroke);

        // Layer 1: animated concentric rings with phase offset.
        for (int ring = 0; ring < 6; ++ring) {
            const float frac = (float)(ring + 1) / 6.0f;
            const float ringR = radius * (0.28f + frac * 0.78f);
            const float ringPulse = 0.55f + 0.45f * std::sinf(timeSec * (1.6f + frac * 0.7f) + frac * 5.0f);
            const int alpha = ClampByte((int)(stroke.GetA() * (0.08f + 0.11f * frac) * ringPulse));
            const float width = 1.3f + frac * 2.4f;
            Gdiplus::Pen pen(Gdiplus::Color((BYTE)alpha, stroke.GetR(), stroke.GetG(), stroke.GetB()), width);
            pen.SetLineJoin(Gdiplus::LineJoinRound);
            g.DrawEllipse(&pen, cx - ringR, cy - ringR, ringR * 2.0f, ringR * 2.0f);
        }

        // Layer 2: dense segmented arcs to create heavy CPU baseline.
        for (int band = 0; band < 4; ++band) {
            const float frac = (float)(band + 1) / 4.0f;
            const float arcR = radius * (0.34f + frac * 0.62f);
            const float spin = timeSec * (30.0f + 18.0f * frac) * ((band & 1) ? -1.0f : 1.0f);
            const int segCount = 26 + band * 8;
            const float sweep = 360.0f / (float)segCount;
            for (int s = 0; s < segCount; ++s) {
                const float phase = timeSec * 2.4f + (float)s * 0.45f;
                const int alpha = ClampByte((int)(stroke.GetA() * (0.16f + 0.18f * std::fabs(std::sinf(phase)))));
                Gdiplus::Pen pen(Gdiplus::Color((BYTE)alpha, stroke.GetR(), stroke.GetG(), stroke.GetB()), 1.2f + frac * 1.8f);
                const float start = spin + s * sweep;
                g.DrawArc(&pen, cx - arcR, cy - arcR, arcR * 2.0f, arcR * 2.0f, start, sweep * 0.62f);
            }
        }

        // Layer 3: orbit particles and connect lines.
        for (size_t i = 0; i < particles_.size(); ++i) {
            OrbitParticle& p = particles_[i];
            const float wobble = std::sinf(timeSec * 1.8f + p.pulse * 9.0f) * radius * 0.045f;
            const float r = p.radius * (0.72f + progress * 0.42f) + wobble;
            const float a = p.angle + timeSec * p.speed * ((i & 1) ? 1.0f : -1.0f);
            const float x = cx + std::cos(a) * r;
            const float y = cy + std::sin(a * 1.15f) * r * 0.84f;

            const int alphaDot = ClampByte((int)(stroke.GetA() * (0.36f + 0.44f * (0.5f + 0.5f * std::sinf(a + timeSec)))));
            Gdiplus::SolidBrush dot(Gdiplus::Color((BYTE)alphaDot, stroke.GetR(), stroke.GetG(), stroke.GetB()));
            const float dotR = 1.3f + 2.4f * (0.5f + 0.5f * std::sinf(timeSec * 2.0f + p.pulse * 7.0f));
            g.FillEllipse(&dot, x - dotR, y - dotR, dotR * 2.0f, dotR * 2.0f);

            if ((i % 2) == 0) {
                const int alphaLine = ClampByte((int)(stroke.GetA() * 0.12f));
                Gdiplus::Pen line(Gdiplus::Color((BYTE)alphaLine, stroke.GetR(), stroke.GetG(), stroke.GetB()), 1.0f);
                g.DrawLine(&line, cx, cy, x, y);
            }
        }

        // Layer 4: center glow.
        const float coreR = style.strokeWidth * (2.6f + progress * 1.8f);
        Gdiplus::GraphicsPath corePath;
        corePath.AddEllipse(cx - coreR, cy - coreR, coreR * 2.0f, coreR * 2.0f);
        Gdiplus::PathGradientBrush core(&corePath);
        core.SetCenterColor(Gdiplus::Color((BYTE)ClampByte((int)(stroke.GetA() * 0.55f)), 255, 255, 255));
        Gdiplus::Color edge[] = { Gdiplus::Color(0, stroke.GetR(), stroke.GetG(), stroke.GetB()) };
        int edgeCount = 1;
        core.SetSurroundColors(edge, &edgeCount);
        g.FillPath(&core, &corePath);
    }

private:
    float ComputeProgress(float t01, uint64_t elapsedMs, uint32_t defaultThresholdMs) {
        using namespace render_utils;
        const uint32_t threshold = thresholdMs_ ? thresholdMs_ : defaultThresholdMs;
        if (threshold == 0) return Clamp01(t01);

        if (currentHoldMs_ > 0 && !holdBiasValid_) {
            holdBiasMs_ = (int64_t)currentHoldMs_ - (int64_t)elapsedMs;
            holdBiasValid_ = true;
        }

        int64_t effectiveMs = (int64_t)elapsedMs;
        if (holdBiasValid_) effectiveMs += holdBiasMs_;
        if (effectiveMs < 0) effectiveMs = 0;
        return Clamp01((float)effectiveMs / (float)threshold);
    }

    std::vector<OrbitParticle> particles_{};
    uint32_t currentHoldMs_ = 0;
    uint32_t thresholdMs_ = 0;
    int64_t holdBiasMs_ = 0;
    bool holdBiasValid_ = false;
};

REGISTER_RENDERER("hold_fluxfield_cpu", FluxFieldHudCpuRenderer)

} // namespace mousefx
