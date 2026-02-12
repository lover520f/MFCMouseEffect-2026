#include "pch.h"

#include "TrailOverlayLayer.h"

namespace mousefx {
namespace {

constexpr int kTrailScreenCullPadding = 220;
constexpr uint64_t kCursorFallbackSampleIntervalMs = 24;
constexpr uint64_t kCursorFallbackSampleIntervalLatencyMs = 32;
constexpr int kTrailLatencyPriorityDurationCapMs = 180;
constexpr int kTrailLatencyPriorityMaxPointsCap = 24;

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
    lastCursorFallbackSampleMs_ = 0;
}

void TrailOverlayLayer::SetLatencyPriorityMode(bool enabled) {
    if (latencyPriorityMode_ == enabled) return;
    latencyPriorityMode_ = enabled;
}

void TrailOverlayLayer::Update(uint64_t nowMs) {
    SampleCursorPoint(nowMs);
    const int effectiveDurationMs = latencyPriorityMode_
        ? (std::min)(durationMs_, kTrailLatencyPriorityDurationCapMs)
        : durationMs_;
    while (!points_.empty()) {
        if (nowMs - points_.front().addedTime > (uint64_t)effectiveDurationMs) {
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

void TrailOverlayLayer::AppendGpuCommands(gpu::OverlayGpuCommandStream& stream, uint64_t nowMs) const {
    if (points_.size() < 2) return;

    gpu::OverlayGpuCommand cmd{};
    cmd.type = gpu::OverlayGpuCommandType::TrailPolyline;
    cmd.effectTag = "trail";
    cmd.flags = isChromatic_ ? gpu::OverlayGpuCommandFlags::kTrailChromatic : 0u;
    cmd.param0 = (float)durationMs_;
    cmd.vertices.reserve(points_.size());

    const uint32_t baseColor = ((uint32_t)color_.GetA() << 24) |
                               ((uint32_t)color_.GetR() << 16) |
                               ((uint32_t)color_.GetG() << 8) |
                               (uint32_t)color_.GetB();
    for (const auto& point : points_) {
        gpu::OverlayGpuVertex v{};
        v.x = (float)point.pt.x;
        v.y = (float)point.pt.y;
        const uint64_t ageMs = (nowMs >= point.addedTime) ? (nowMs - point.addedTime) : 0;
        const float life = (durationMs_ > 0)
            ? (1.0f - (float)ageMs / (float)durationMs_)
            : 1.0f;
        v.extra = (life < 0.0f) ? 0.0f : ((life > 1.0f) ? 1.0f : life);
        v.colorArgb = baseColor;
        cmd.vertices.push_back(v);
    }

    if (!cmd.vertices.empty()) {
        stream.Add(std::move(cmd));
    }
}

void TrailOverlayLayer::SampleCursorPoint(uint64_t nowMs) {
    POINT pt{};
    bool havePoint = false;
    if (hasLatestCursorPt_) {
        pt = latestCursorPt_;
        hasLatestCursorPt_ = false;
        havePoint = true;
    } else {
        const uint64_t fallbackIntervalMs = latencyPriorityMode_
            ? kCursorFallbackSampleIntervalLatencyMs
            : kCursorFallbackSampleIntervalMs;
        const uint64_t elapsed = (nowMs >= lastCursorFallbackSampleMs_)
            ? (nowMs - lastCursorFallbackSampleMs_)
            : 0;
        if (lastCursorFallbackSampleMs_ == 0 || elapsed >= fallbackIntervalMs) {
            if (GetCursorPos(&pt)) {
                havePoint = true;
                lastCursorFallbackSampleMs_ = nowMs;
            }
        }
    }
    if (!havePoint) return;

    if (hasLastSamplePt_ && pt.x == lastSamplePt_.x && pt.y == lastSamplePt_.y) {
        return;
    }

    TrailPoint trailPoint{};
    trailPoint.pt = pt;
    trailPoint.addedTime = nowMs;
    points_.push_back(trailPoint);
    const size_t effectiveMaxPoints = (size_t)(latencyPriorityMode_
        ? (std::min)(maxPoints_, kTrailLatencyPriorityMaxPointsCap)
        : maxPoints_);
    if (points_.size() > effectiveMaxPoints) {
        points_.pop_front();
    }
    lastSamplePt_ = pt;
    hasLastSamplePt_ = true;
}

} // namespace mousefx
