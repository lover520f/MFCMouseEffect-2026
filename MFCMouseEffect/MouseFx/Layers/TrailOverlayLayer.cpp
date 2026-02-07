#include "pch.h"

#include "TrailOverlayLayer.h"

namespace mousefx {

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

void TrailOverlayLayer::AddPoint(const POINT& pt) {
    latestCursorPt_ = pt;
    hasLatestCursorPt_ = true;
}

void TrailOverlayLayer::Clear() {
    points_.clear();
    hasLastSamplePt_ = false;
}

void TrailOverlayLayer::Update(uint64_t nowMs) {
    SampleCursorPoint(nowMs);
    while (!points_.empty()) {
        if (nowMs - points_.front().addedTime > (uint64_t)durationMs_) {
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
    POINT pt{};
    bool havePoint = false;
    if (hasLatestCursorPt_) {
        pt = latestCursorPt_;
        hasLatestCursorPt_ = false;
        havePoint = true;
    } else if (GetCursorPos(&pt)) {
        havePoint = true;
    }
    if (!havePoint) return;

    if (hasLastSamplePt_ && pt.x == lastSamplePt_.x && pt.y == lastSamplePt_.y) {
        return;
    }

    TrailPoint trailPoint{};
    trailPoint.pt = pt;
    trailPoint.addedTime = nowMs;
    points_.push_back(trailPoint);
    if (points_.size() > (size_t)maxPoints_) {
        points_.pop_front();
    }
    lastSamplePt_ = pt;
    hasLastSamplePt_ = true;
}

} // namespace mousefx
