#pragma once

#include "MouseFx/Interfaces/IOverlayLayer.h"
#include "MouseFx/Interfaces/ITrailRenderer.h"

#include <deque>
#include <memory>

namespace mousefx {

class TrailOverlayLayer final : public IOverlayLayer {
public:
    TrailOverlayLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, Gdiplus::Color color, bool isChromatic);

    void AddPoint(const TrailPoint& point);
    void Clear();

    void Update(uint64_t nowMs) override;
    void Render(Gdiplus::Graphics& graphics) override;
    bool IsAlive() const override { return true; }

private:
    void SampleCursorPoint(uint64_t nowMs);

    std::deque<TrailPoint> points_{};
    std::unique_ptr<ITrailRenderer> renderer_{};
    int durationMs_ = 350;
    int maxPoints_ = 40;
    Gdiplus::Color color_{220, 100, 255, 218};
    bool isChromatic_ = false;
    TrailPoint latestPoint_{}; 
    bool hasLatestPoint_ = false;
    uint64_t lastInputPointTickMs_ = 0;
    ScreenPoint lastSamplePt_{};
    bool hasLastSamplePt_ = false;
};

} // namespace mousefx
