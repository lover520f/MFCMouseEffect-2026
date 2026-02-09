#include "pch.h"

#include "TrailOverlayLayer.h"

namespace mousefx {
namespace {

constexpr int kTrailScreenCullPadding = 220;

bool RectanglesOverlap(int l1, int t1, int r1, int b1, int l2, int t2, int r2, int b2) {
    return !(r1 <= l2 || r2 <= l1 || b1 <= t2 || b2 <= t1);
}

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

bool TrailOverlayLayer::IntersectsScreenRect(int left, int top, int right, int bottom) const {
    if (left >= right || top >= bottom) return false;

    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    bool hasBounds = false;

    if (!points_.empty()) {
        minX = maxX = points_.front().pt.x;
        minY = maxY = points_.front().pt.y;
        for (const auto& point : points_) {
            if (point.pt.x < minX) minX = point.pt.x;
            if (point.pt.y < minY) minY = point.pt.y;
            if (point.pt.x > maxX) maxX = point.pt.x;
            if (point.pt.y > maxY) maxY = point.pt.y;
        }
        hasBounds = true;
    } else if (hasLastSamplePt_) {
        minX = maxX = lastSamplePt_.x;
        minY = maxY = lastSamplePt_.y;
        hasBounds = true;
    } else if (hasLatestCursorPt_) {
        minX = maxX = latestCursorPt_.x;
        minY = maxY = latestCursorPt_.y;
        hasBounds = true;
    }

    if (!hasBounds) return false;

    minX -= kTrailScreenCullPadding;
    minY -= kTrailScreenCullPadding;
    maxX += kTrailScreenCullPadding;
    maxY += kTrailScreenCullPadding;
    return RectanglesOverlap(left, top, right, bottom, minX, minY, maxX + 1, maxY + 1);
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
