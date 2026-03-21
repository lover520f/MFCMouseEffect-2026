#include "pch.h"

#include "TrailOverlayLayer.h"
#include "MouseFx/Core/System/CursorPositionProvider.h"

namespace mousefx {
namespace {

constexpr uint64_t kTrailSyntheticFollowWindowMs = 48;

} // namespace

TrailOverlayLayer::TrailOverlayLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, Gdiplus::Color color, bool isChromatic)
    : renderer_(std::move(renderer)),
      durationMs_(durationMs),
      maxPoints_(maxPoints),
      color_(color),
      isChromatic_(isChromatic) {
    if (durationMs_ < 80) durationMs_ = 80;
    if (durationMs_ > 2000) durationMs_ = 2000;
    if (maxPoints_ < 2) maxPoints_ = 2;
    if (maxPoints_ > 240) maxPoints_ = 240;
}

void TrailOverlayLayer::AddPoint(const TrailPoint& point) {
    latestPoint_ = point;
    hasLatestPoint_ = true;
    lastInputPointTickMs_ = point.addedTime;
}

void TrailOverlayLayer::Clear() {
    points_.clear();
    hasLastSamplePt_ = false;
    lastInputPointTickMs_ = 0;
}

void TrailOverlayLayer::Update(uint64_t nowMs) {
    SampleCursorPoint(nowMs);
    while (!points_.empty()) {
        const int pointDurationMs = trail_point_style::ResolveDurationMs(points_.front(), durationMs_);
        if (nowMs - points_.front().addedTime > static_cast<uint64_t>(pointDurationMs)) {
            points_.pop_front();
        } else {
            break;
        }
    }
}

void TrailOverlayLayer::Render(Gdiplus::Graphics& graphics) {
    if (!renderer_) return;
    const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    renderer_->Render(graphics, points_, width, height, color_, isChromatic_);
}

void TrailOverlayLayer::SampleCursorPoint(uint64_t nowMs) {
    TrailPoint incoming{};
    ScreenPoint pt{};
    bool havePoint = false;
    if (hasLatestPoint_) {
        incoming = latestPoint_;
        pt = incoming.pt;
        hasLatestPoint_ = false;
        havePoint = true;
    } else if (lastInputPointTickMs_ != 0 &&
               nowMs >= lastInputPointTickMs_ &&
               (nowMs - lastInputPointTickMs_) <= kTrailSyntheticFollowWindowMs &&
               TryGetCursorScreenPoint(&pt)) {
        havePoint = true;
    }
    if (!havePoint) return;

    if (hasLastSamplePt_ && pt.x == lastSamplePt_.x && pt.y == lastSamplePt_.y) {
        return;
    }

    TrailPoint trailPoint = incoming;
    trailPoint.pt = pt;
    trailPoint.addedTime = nowMs;
    if (trailPoint.durationMs <= 0) {
        trailPoint.durationMs = durationMs_;
    }
    if (trailPoint.lineWidthPx <= 0.0) {
        trailPoint.lineWidthPx = 4.0;
    }
    if (trailPoint.strokeArgb == 0) {
        trailPoint.strokeArgb = (0xFFu << 24) |
            (static_cast<uint32_t>(color_.GetR()) << 16) |
            (static_cast<uint32_t>(color_.GetG()) << 8) |
            static_cast<uint32_t>(color_.GetB());
    }
    if (trailPoint.fillArgb == 0) {
        trailPoint.fillArgb = (0x66u << 24) | (trailPoint.strokeArgb & 0x00FFFFFFu);
    }
    points_.push_back(trailPoint);
    if (points_.size() > (size_t)maxPoints_) {
        points_.pop_front();
    }
    lastSamplePt_ = pt;
    hasLastSamplePt_ = true;
}

} // namespace mousefx
