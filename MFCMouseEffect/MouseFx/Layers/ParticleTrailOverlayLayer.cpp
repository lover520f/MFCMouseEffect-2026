#include "pch.h"

#include "ParticleTrailOverlayLayer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

#include <algorithm>
#include <cmath>

namespace mousefx {

ParticleTrailOverlayLayer::ParticleTrailOverlayLayer(bool isChromatic) : isChromatic_(isChromatic) {}

void ParticleTrailOverlayLayer::UpdateCursor(const POINT& pt) {
    latestCursorPt_ = pt;
    hasLatestCursorPt_ = true;
}

void ParticleTrailOverlayLayer::Clear() {
    particles_.clear();
    hasLastEmitCursorPt_ = false;
}

void ParticleTrailOverlayLayer::Update(uint64_t nowMs) {
    float dt = 0.016f;
    if (lastTickMs_ != 0 && nowMs >= lastTickMs_) {
        dt = (float)(nowMs - lastTickMs_) / 1000.0f;
        if (dt > 0.1f) dt = 0.1f;
    }
    lastTickMs_ = nowMs;

    POINT pt{};
    bool havePt = false;
    if (hasLatestCursorPt_) {
        pt = latestCursorPt_;
        hasLatestCursorPt_ = false;
        havePt = true;
    } else if (GetCursorPos(&pt)) {
        havePt = true;
    }

    if (havePt) {
        if (!hasLastEmitCursorPt_) {
            lastEmitCursorPt_ = pt;
            hasLastEmitCursorPt_ = true;
        }
        const float dx = (float)(pt.x - lastEmitCursorPt_.x);
        const float dy = (float)(pt.y - lastEmitCursorPt_.y);
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist >= 1.0f) {
            int emitCount = (int)(dist * 0.18f) + 2;
            if (emitCount < 2) emitCount = 2;
            if (emitCount > 12) emitCount = 12;
            Emit(pt, emitCount);
            lastEmitCursorPt_ = pt;
        }
    }

    for (auto it = particles_.begin(); it != particles_.end();) {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.05f;
        it->life -= dt * 1.5f;
        if (it->life <= 0.0f) {
            it = particles_.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleTrailOverlayLayer::Render(Gdiplus::Graphics& graphics) {
    if (particles_.empty()) return;
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    for (const auto& particle : particles_) {
        ScreenPoint screenPt{};
        screenPt.x = static_cast<int32_t>(std::lround(particle.x));
        screenPt.y = static_cast<int32_t>(std::lround(particle.y));
        const ScreenPoint localPt = ScreenToOverlayPoint(screenPt);

        BYTE alpha = (BYTE)(particle.life * 255.0f);
        Gdiplus::Color color = HslToRgb(particle.hue, 0.8f, 0.6f, alpha);
        Gdiplus::SolidBrush brush(color);
        float size = particle.size * particle.life;
        graphics.FillEllipse(&brush, (float)localPt.x - size * 0.5f, (float)localPt.y - size * 0.5f, size, size);
    }
}

Gdiplus::Color ParticleTrailOverlayLayer::HslToRgb(float h, float s, float l, BYTE alpha) {
    float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c * 0.5f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    if (h < 60.0f) {
        r = c; g = x;
    } else if (h < 120.0f) {
        r = x; g = c;
    } else if (h < 180.0f) {
        g = c; b = x;
    } else if (h < 240.0f) {
        g = x; b = c;
    } else if (h < 300.0f) {
        r = x; b = c;
    } else {
        r = c; b = x;
    }

    return Gdiplus::Color(alpha,
        (BYTE)((r + m) * 255.0f),
        (BYTE)((g + m) * 255.0f),
        (BYTE)((b + m) * 255.0f));
}

void ParticleTrailOverlayLayer::Emit(const POINT& pt, int count) {
    globalHue_ = std::fmod(globalHue_ + 5.0f, 360.0f);

    for (int i = 0; i < count; ++i) {
        Particle particle{};
        // Keep particle positions in screen space.
        // OverlayHost may render the same layer onto multiple per-monitor windows.
        // Converting to local coordinates here can bind points to the wrong monitor origin.
        particle.x = (float)pt.x;
        particle.y = (float)pt.y;

        const float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        const float speed = (float)(rand() % 100) / 30.0f + 0.5f;
        particle.vx = std::cos(angle) * speed;
        particle.vy = std::sin(angle) * speed;
        particle.life = 1.0f;
        particle.size = (float)(rand() % 40) / 10.0f + 2.0f;

        if (isChromatic_) {
            particle.hue = (float)(rand() % 360);
        } else {
            particle.hue = globalHue_ + (float)(rand() % 40 - 20);
        }

        particles_.push_back(particle);
    }
}

} // namespace mousefx
