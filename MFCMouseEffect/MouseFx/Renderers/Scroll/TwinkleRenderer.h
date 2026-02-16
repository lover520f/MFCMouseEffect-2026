#pragma once

#include "../RenderUtils.h"
#include "../RendererRegistry.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace mousefx {

class TwinkleRenderer : public IRippleRenderer {
public:
    void SetParams(const RenderParams& params) override { params_ = params; }

    void Start(const RippleStyle& style) override {
        (void)style;
        particles_.clear();
        started_ = true;
        emitted_ = false;
        lastElapsedMs_ = 0;
        rngState_ ^= NextSeed();
        if (rngState_ == 0u) {
            rngState_ = 0xA341316Cu;
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        using namespace render_utils;

        if (!started_) {
            Start(style);
        }

        const float tn = Clamp01(t);
        const float intensity = Clamp01(params_.intensity);
        const float lifeFade = 1.0f - Smoothstep(0.70f, 1.0f, tn);
        if (lifeFade <= 0.001f) {
            particles_.clear();
            return;
        }

        const float cx = sizePx * 0.5f;
        const float cy = sizePx * 0.5f;
        const float dir = params_.directionRad;
        const float dirX = std::cos(dir);
        const float dirY = std::sin(dir);
        const float prpX = -dirY;
        const float prpY = dirX;

        if (!emitted_) {
            EmitBurst(cx, cy, dirX, dirY, prpX, prpY, intensity);
            emitted_ = true;
        }
        UpdateParticles(elapsedMs, dirX, dirY, prpX, prpY, sizePx);

        const Gdiplus::Color stroke = ToGdiPlus(style.stroke);
        const Gdiplus::Color glow = ToGdiPlus(style.glow);
        const Gdiplus::Color white(255, 255, 255, 255);
        const float baseSize = 0.78f + 1.20f * intensity;
        const float seconds = static_cast<float>(elapsedMs) / 1000.0f;

        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
        for (const Particle& p : particles_) {
            const float life = 1.0f - Clamp01(p.ageMs / p.lifeMs);
            if (life <= 0.0f) {
                continue;
            }

            const float flicker = 0.86f + 0.14f * std::sin(seconds * 9.0f + p.flicker);
            const float drawSize = baseSize * p.size * (0.55f + 0.45f * life) * flicker;
            if (drawSize <= 0.08f) {
                continue;
            }

            const BYTE coreAlpha = ClampByte(static_cast<int>(178.0f * life * lifeFade));
            if (coreAlpha <= 2) {
                continue;
            }

            const Gdiplus::Color coreColor = MixColor(
                stroke,
                white,
                0.05f + (1.0f - life) * 0.14f,
                coreAlpha);
            const BYTE trailAlpha = ClampByte(static_cast<int>(coreAlpha * 0.64f));
            const Gdiplus::Color trailColor = MixColor(glow, coreColor, 0.38f, trailAlpha);

            Gdiplus::Pen trailPen(trailColor, std::max(0.75f, drawSize * 0.26f));
            trailPen.SetStartCap(Gdiplus::LineCapRound);
            trailPen.SetEndCap(Gdiplus::LineCapRound);
            g.DrawLine(&trailPen, p.prevX, p.prevY, p.x, p.y);

            const BYTE glowAlpha = ClampByte(static_cast<int>(coreAlpha * 0.14f));
            const Gdiplus::Color glowColor = MixColor(glow, coreColor, 0.20f, glowAlpha);
            const float glowSize = drawSize * 1.22f;
            Gdiplus::SolidBrush glowBrush(glowColor);
            g.FillEllipse(
                &glowBrush,
                p.x - glowSize * 0.5f,
                p.y - glowSize * 0.5f,
                glowSize,
                glowSize);

            Gdiplus::SolidBrush coreBrush(coreColor);
            g.FillEllipse(
                &coreBrush,
                p.x - drawSize * 0.5f,
                p.y - drawSize * 0.5f,
                drawSize,
                drawSize);
        }
    }

private:
    struct Particle {
        float x = 0.0f;
        float y = 0.0f;
        float prevX = 0.0f;
        float prevY = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float size = 0.0f;
        float ageMs = 0.0f;
        float lifeMs = 1.0f;
        float flicker = 0.0f;
    };

    static constexpr float kConeHalfRad = 3.1415926535f / 6.0f; // 30 deg, total cone 60 deg.

    static float Smoothstep(float a, float b, float x) {
        if (b <= a) {
            return x >= b ? 1.0f : 0.0f;
        }
        float u = (x - a) / (b - a);
        if (u < 0.0f) u = 0.0f;
        if (u > 1.0f) u = 1.0f;
        return u * u * (3.0f - 2.0f * u);
    }

    static Gdiplus::Color MixColor(const Gdiplus::Color& a, const Gdiplus::Color& b, float t, BYTE alpha) {
        using namespace render_utils;
        const float k = Clamp01(t);
        const int r = static_cast<int>(std::lround(static_cast<float>(a.GetR()) * (1.0f - k) + static_cast<float>(b.GetR()) * k));
        const int g = static_cast<int>(std::lround(static_cast<float>(a.GetG()) * (1.0f - k) + static_cast<float>(b.GetG()) * k));
        const int bl = static_cast<int>(std::lround(static_cast<float>(a.GetB()) * (1.0f - k) + static_cast<float>(b.GetB()) * k));
        return Gdiplus::Color(alpha, ClampByte(r), ClampByte(g), ClampByte(bl));
    }

    static uint32_t NextSeed() {
        static uint32_t seed = 0xC13FA9A9u;
        seed = seed * 1664525u + 1013904223u;
        return seed;
    }

    float Rand01() {
        rngState_ ^= (rngState_ << 13);
        rngState_ ^= (rngState_ >> 17);
        rngState_ ^= (rngState_ << 5);
        return static_cast<float>(rngState_ & 0x00FFFFFFu) / 16777216.0f;
    }

    void EmitBurst(float cx, float cy, float dirX, float dirY, float prpX, float prpY, float intensity) {
        const int count = static_cast<int>(10.0f + intensity * 12.0f);
        particles_.reserve(static_cast<size_t>(count));

        for (int i = 0; i < count; ++i) {
            const float coneOffset = (Rand01() * 2.0f - 1.0f) * kConeHalfRad;
            const float c = std::cos(coneOffset);
            const float s = std::sin(coneOffset);
            const float sprayX = dirX * c + prpX * s;
            const float sprayY = dirY * c + prpY * s;
            const float sideJitter = (Rand01() * 2.0f - 1.0f) * (1.4f + 3.0f * intensity);
            const float backJitter = Rand01() * (2.5f + 5.0f * intensity);
            const float speed = 4.2f + Rand01() * (4.8f + 2.2f * intensity);

            Particle p{};
            p.x = cx + prpX * sideJitter - dirX * backJitter;
            p.y = cy + prpY * sideJitter - dirY * backJitter;
            p.prevX = p.x;
            p.prevY = p.y;
            p.vx = sprayX * speed;
            p.vy = sprayY * speed;
            p.size = 1.0f + Rand01() * 1.4f;
            p.ageMs = 0.0f;
            p.lifeMs = 95.0f + Rand01() * 120.0f;
            p.flicker = Rand01() * 6.2831853f;
            particles_.push_back(p);
        }
    }

    void UpdateParticles(uint64_t elapsedMs, float dirX, float dirY, float prpX, float prpY, int sizePx) {
        uint64_t dtMs = (lastElapsedMs_ == 0 || elapsedMs <= lastElapsedMs_) ? 16u : (elapsedMs - lastElapsedMs_);
        if (dtMs > 34u) dtMs = 34u;
        lastElapsedMs_ = elapsedMs;

        const float step = static_cast<float>(dtMs) / 16.0f;
        const float drag = std::pow(0.90f, step);
        const float fieldLeft = -sizePx * 0.5f;
        const float fieldRight = sizePx * 1.5f;
        const float fieldTop = -sizePx * 0.5f;
        const float fieldBottom = sizePx * 1.5f;
        const float flow = 0.015f * step;
        const float turbulence = 0.040f * step;

        for (size_t i = 0; i < particles_.size();) {
            Particle& p = particles_[i];

            p.ageMs += static_cast<float>(dtMs);
            if (p.ageMs >= p.lifeMs) {
                particles_[i] = particles_.back();
                particles_.pop_back();
                continue;
            }

            p.prevX = p.x;
            p.prevY = p.y;
            p.vx = p.vx * drag + dirX * flow + prpX * ((Rand01() * 2.0f - 1.0f) * turbulence);
            p.vy = p.vy * drag + dirY * flow + (Rand01() * 2.0f - 1.0f) * turbulence;
            p.x += p.vx * step;
            p.y += p.vy * step;
            p.flicker += 0.16f * step;

            if (p.x < fieldLeft || p.x > fieldRight ||
                p.y < fieldTop || p.y > fieldBottom) {
                particles_[i] = particles_.back();
                particles_.pop_back();
                continue;
            }
            ++i;
        }
    }

    RenderParams params_{};
    std::vector<Particle> particles_{};
    bool started_ = false;
    bool emitted_ = false;
    uint64_t lastElapsedMs_ = 0;
    uint32_t rngState_ = 0x9A53C11Du;
};

REGISTER_RENDERER("twinkle", TwinkleRenderer)

} // namespace mousefx
