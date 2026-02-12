#pragma once

#include "MouseFx/Interfaces/IOverlayLayer.h"

#include <vector>

namespace mousefx {

class ParticleTrailOverlayLayer final : public IOverlayLayer {
public:
    explicit ParticleTrailOverlayLayer(bool isChromatic);

    void UpdateCursor(const POINT& pt);
    void Clear();

    void Update(uint64_t nowMs) override;
    void Render(Gdiplus::Graphics& graphics) override;
    bool IsAlive() const override { return true; }
    bool SupportsGpuExclusivePresent() const override { return true; }
    bool IntersectsScreenRect(int left, int top, int right, int bottom) const override;
    void AppendGpuCommands(gpu::OverlayGpuCommandStream& stream, uint64_t nowMs) const override;

private:
    struct Particle {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float life = 1.0f;
        float hue = 0.0f;
        float size = 1.0f;
    };

    static Gdiplus::Color HslToRgb(float h, float s, float l, BYTE alpha);
    void Emit(const POINT& pt, int count);

    bool isChromatic_ = false;
    std::vector<Particle> particles_{};
    uint64_t lastTickMs_ = 0;
    POINT latestCursorPt_{};
    bool hasLatestCursorPt_ = false;
    POINT lastEmitCursorPt_{};
    bool hasLastEmitCursorPt_ = false;
    float globalHue_ = 0.0f;
};

} // namespace mousefx
