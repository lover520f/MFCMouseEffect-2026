#pragma once

#include "MouseFx/Interfaces/IOverlayLayer.h"
#include "MouseFx/Interfaces/ITrailRenderer.h"

#include <deque>
#include <memory>

namespace mousefx {

class TrailOverlayLayer final : public IOverlayLayer {
public:
    TrailOverlayLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, Gdiplus::Color color, bool isChromatic);

    void AddPoint(const POINT& pt);
    void Clear();

    void Update(uint64_t nowMs) override;
    void Render(Gdiplus::Graphics& graphics) override;
    bool IsAlive() const override { return true; }
    bool SupportsGpuExclusivePresent() const override { return true; }
    bool IntersectsScreenRect(int left, int top, int right, int bottom) const override;
    void AppendGpuCommands(gpu::OverlayGpuCommandStream& stream, uint64_t nowMs) const override;

private:
    void SampleCursorPoint(uint64_t nowMs);

    std::deque<TrailPoint> points_{};
    std::unique_ptr<ITrailRenderer> renderer_{};
    int durationMs_ = 350;
    int maxPoints_ = 40;
    Gdiplus::Color color_{220, 100, 255, 218};
    bool isChromatic_ = false;
    POINT latestCursorPt_{};
    bool hasLatestCursorPt_ = false;
    POINT lastSamplePt_{};
    bool hasLastSamplePt_ = false;
};

} // namespace mousefx
