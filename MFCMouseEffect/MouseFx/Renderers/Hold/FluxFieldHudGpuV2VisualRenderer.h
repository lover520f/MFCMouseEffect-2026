#pragma once

#include "../RenderUtils.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace mousefx {

class FluxFieldHudGpuV2VisualRenderer final {
public:
    struct FluxNode {
        float radius = 0.0f;
        float angle = 0.0f;
        float speed = 0.0f;
        float phase = 0.0f;
        float wobble = 0.0f;
    };

    void Start(const RippleStyle& style) {
        currentHoldMs_ = 0;
        thresholdMs_ = style.durationMs;
        holdBiasMs_ = 0;
        holdBiasValid_ = false;
        nodes_.clear();

        const float base = (style.endRadius > 0.0f) ? style.endRadius : 96.0f;
        const int nodeCount = 56;
        nodes_.reserve(nodeCount);
        for (int i = 0; i < nodeCount; ++i) {
            FluxNode n{};
            const float r0 = Hash01(static_cast<uint32_t>(i * 31 + 7));
            const float r1 = Hash01(static_cast<uint32_t>(i * 47 + 13));
            const float r2 = Hash01(static_cast<uint32_t>(i * 59 + 29));
            const float r3 = Hash01(static_cast<uint32_t>(i * 71 + 41));
            const float r4 = Hash01(static_cast<uint32_t>(i * 83 + 53));
            n.radius = base * (0.18f + r0 * 0.82f);
            n.angle = r1 * 6.2831853f;
            n.speed = 0.45f + r2 * 1.35f;
            n.phase = r3 * 6.2831853f;
            n.wobble = base * (0.02f + r4 * 0.08f);
            nodes_.push_back(n);
        }
    }

    void OnCommand(const std::string& cmd, const std::string& args) {
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

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) {
        using namespace render_utils;
        if (nodes_.empty()) Start(style);

        const float cx = sizePx * 0.5f;
        const float cy = sizePx * 0.5f;
        const float progress = ComputeProgress(t, elapsedMs, style.durationMs);
        const float timeSec = static_cast<float>(elapsedMs) / 1000.0f;
        const float radius = style.startRadius + (style.endRadius - style.startRadius) * progress;
        const Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        const Gdiplus::Color glow = ToGdiPlus(style.glow);

        // Layer 1: segmented rotating ribbons (not full circles).
        for (int band = 0; band < 3; ++band) {
            const float bandFrac = static_cast<float>(band + 1) / 3.0f;
            const float arcBase = radius * (0.28f + bandFrac * 0.54f);
            const float spin = timeSec * (28.0f + bandFrac * 22.0f) * ((band & 1) ? -1.0f : 1.0f);
            const int segCount = 14 + band * 8;
            const float sweep = 360.0f / static_cast<float>(segCount);
            for (int seg = 0; seg < segCount; ++seg) {
                const float segPulse = 0.45f + 0.55f * std::fabs(std::sinf(timeSec * 2.2f + seg * 0.37f + band * 1.13f));
                const float arcR = arcBase + std::sinf(timeSec * (1.3f + bandFrac) + seg * 0.29f) * radius * 0.055f;
                const int alpha = ClampByte(static_cast<int>(stroke.GetA() * (0.07f + 0.20f * segPulse)));
                const float width = 1.3f + bandFrac * 1.9f;
                Gdiplus::Pen pen(Gdiplus::Color(static_cast<BYTE>(alpha), stroke.GetR(), stroke.GetG(), stroke.GetB()), width);
                const float start = spin + seg * sweep + std::sinf(timeSec + seg * 0.25f) * 8.0f;
                const float span = sweep * (0.23f + 0.50f * segPulse);
                g.DrawArc(&pen, cx - arcR, cy - arcR, arcR * 2.0f, arcR * 2.0f, start, span);
            }
        }

        // Layer 2: particle flow and short filament links.
        std::vector<Gdiplus::PointF> points;
        points.reserve(nodes_.size());
        for (size_t i = 0; i < nodes_.size(); ++i) {
            FluxNode& n = nodes_[i];
            const float spinSign = (i & 1u) ? 1.0f : -1.0f;
            const float angle = n.angle + timeSec * n.speed * spinSign;
            const float radialPulse = std::sinf(timeSec * 2.0f + n.phase);
            const float orbitScale = 0.52f + progress * 0.64f;
            const float r = n.radius * orbitScale + radialPulse * n.wobble;
            const float yScale = 0.78f + 0.16f * std::sinf(timeSec * 0.9f + n.phase);
            const float x = cx + std::cos(angle) * r;
            const float y = cy + std::sin(angle * 1.08f) * r * yScale;
            points.emplace_back(x, y);

            const float dotPulse = 0.5f + 0.5f * std::sinf(timeSec * 1.8f + n.phase);
            const int alpha = ClampByte(static_cast<int>(stroke.GetA() * (0.24f + 0.50f * dotPulse)));
            const float dotR = 1.0f + 2.2f * dotPulse;
            Gdiplus::SolidBrush dotBrush(Gdiplus::Color(static_cast<BYTE>(alpha), stroke.GetR(), stroke.GetG(), stroke.GetB()));
            g.FillEllipse(&dotBrush, x - dotR, y - dotR, dotR * 2.0f, dotR * 2.0f);
        }

        for (size_t i = 1; i < points.size(); ++i) {
            if ((i & 1u) != 0u) continue;
            const float pulse = 0.5f + 0.5f * std::sinf(timeSec * 2.4f + static_cast<float>(i) * 0.33f);
            const int alpha = ClampByte(static_cast<int>(stroke.GetA() * (0.07f + 0.14f * pulse)));
            Gdiplus::Pen filament(
                Gdiplus::Color(static_cast<BYTE>(alpha), stroke.GetR(), stroke.GetG(), stroke.GetB()),
                0.9f + pulse * 0.8f);
            g.DrawLine(&filament, points[i - 1], points[i]);
        }
        for (size_t i = 0; i < points.size(); i += 3) {
            const float pulse = 0.5f + 0.5f * std::sinf(timeSec * 1.5f + static_cast<float>(i) * 0.21f);
            const int alpha = ClampByte(static_cast<int>(stroke.GetA() * (0.05f + 0.11f * pulse)));
            Gdiplus::Pen beam(
                Gdiplus::Color(static_cast<BYTE>(alpha), glow.GetR(), glow.GetG(), glow.GetB()),
                0.9f);
            g.DrawLine(&beam, cx, cy, points[i].X, points[i].Y);
        }

        // Layer 3: shear rays from the core.
        const int rayCount = 20;
        for (int i = 0; i < rayCount; ++i) {
            const float phase = timeSec * 1.4f + i * 0.42f;
            const float a = timeSec * 1.1f + i * (6.2831853f / rayCount) + std::sinf(phase) * 0.22f;
            const float inner = radius * (0.06f + 0.05f * progress);
            const float outer = radius * (0.24f + 0.66f * std::fabs(std::sinf(phase)));
            const float x0 = cx + std::cos(a) * inner;
            const float y0 = cy + std::sin(a) * inner;
            const float x1 = cx + std::cos(a) * outer;
            const float y1 = cy + std::sin(a) * outer;
            const int alpha = ClampByte(static_cast<int>(stroke.GetA() * (0.08f + 0.18f * std::fabs(std::sinf(phase)))));
            Gdiplus::Pen ray(
                Gdiplus::Color(static_cast<BYTE>(alpha), glow.GetR(), glow.GetG(), glow.GetB()),
                1.0f + 0.6f * std::fabs(std::sinf(phase)));
            g.DrawLine(&ray, x0, y0, x1, y1);
        }

        // Layer 4: central glow.
        const float coreR = style.strokeWidth * (2.2f + progress * 2.0f);
        Gdiplus::GraphicsPath corePath;
        corePath.AddEllipse(cx - coreR, cy - coreR, coreR * 2.0f, coreR * 2.0f);
        Gdiplus::PathGradientBrush core(&corePath);
        core.SetCenterColor(Gdiplus::Color(ClampByte(static_cast<int>(stroke.GetA() * 0.52f)), 255, 255, 255));
        Gdiplus::Color edge[] = { Gdiplus::Color(0, glow.GetR(), glow.GetG(), glow.GetB()) };
        int edgeCount = 1;
        core.SetSurroundColors(edge, &edgeCount);
        g.FillPath(&core, &corePath);
    }

private:
    static float Hash01(uint32_t x) {
        x ^= x >> 17;
        x *= 0xed5ad4bbU;
        x ^= x >> 11;
        x *= 0xac4c1b51U;
        x ^= x >> 15;
        x *= 0x31848babU;
        x ^= x >> 14;
        return static_cast<float>(x & 0x00FFFFFFu) / 16777215.0f;
    }

    float ComputeProgress(float t01, uint64_t elapsedMs, uint32_t defaultThresholdMs) {
        using namespace render_utils;
        const uint32_t threshold = thresholdMs_ ? thresholdMs_ : defaultThresholdMs;
        if (threshold == 0) return Clamp01(t01);

        if (currentHoldMs_ > 0 && !holdBiasValid_) {
            holdBiasMs_ = static_cast<int64_t>(currentHoldMs_) - static_cast<int64_t>(elapsedMs);
            holdBiasValid_ = true;
        }

        int64_t effectiveMs = static_cast<int64_t>(elapsedMs);
        if (holdBiasValid_) effectiveMs += holdBiasMs_;
        if (effectiveMs < 0) effectiveMs = 0;
        return Clamp01(static_cast<float>(effectiveMs) / static_cast<float>(threshold));
    }

    std::vector<FluxNode> nodes_{};
    uint32_t currentHoldMs_ = 0;
    uint32_t thresholdMs_ = 0;
    int64_t holdBiasMs_ = 0;
    bool holdBiasValid_ = false;
};

} // namespace mousefx
