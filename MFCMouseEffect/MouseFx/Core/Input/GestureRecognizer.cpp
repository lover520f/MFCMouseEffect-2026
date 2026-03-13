#include "pch.h"
#include "GestureRecognizer.h"

#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>

namespace mousefx {
namespace {
constexpr int kButtonNone = 0;
constexpr int kButtonLeft = 1;
constexpr int kButtonRight = 2;
constexpr int kButtonMiddle = 3;
} // namespace

void GestureRecognizer::UpdateConfig(const GestureRecognitionConfig& config) {
    config_ = config;
    config_.minStrokeDistancePx = ClampInt(config_.minStrokeDistancePx, 10, 4000);
    config_.sampleStepPx = ClampInt(config_.sampleStepPx, 2, 256);
    config_.maxDirections = ClampInt(config_.maxDirections, 1, 8);
    Reset();
}

void GestureRecognizer::Reset() {
    active_ = false;
    activeButton_ = 0;
    totalDistancePx_ = 0;
    samples_.clear();
    sampleTimesMs_.clear();
    lastRawPt_ = ScreenPoint{};
    lastSamplePt_ = ScreenPoint{};
    startedAt_ = {};
    lastRawAt_ = {};
    lastSampleAt_ = {};
}

void GestureRecognizer::OnButtonDown(const ScreenPoint& pt, int button) {
    Reset();
    if (!config_.enabled) {
        return;
    }
    if (!IsTrackedButton(button)) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    active_ = true;
    activeButton_ = button;
    lastRawPt_ = pt;
    lastSamplePt_ = pt;
    startedAt_ = now;
    lastRawAt_ = now;
    lastSampleAt_ = now;
    samples_.push_back(pt);
    sampleTimesMs_.push_back(0);
}

void GestureRecognizer::OnMouseMove(const ScreenPoint& pt) {
    if (!active_) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();

    const long long dxRaw = static_cast<long long>(pt.x) - static_cast<long long>(lastRawPt_.x);
    const long long dyRaw = static_cast<long long>(pt.y) - static_cast<long long>(lastRawPt_.y);
    totalDistancePx_ += static_cast<int>(std::sqrt(static_cast<double>(dxRaw * dxRaw + dyRaw * dyRaw)));
    lastRawPt_ = pt;
    lastRawAt_ = now;

    const int stepSq = config_.sampleStepPx * config_.sampleStepPx;
    if (DistanceSquared(lastSamplePt_, pt) < stepSq) {
        return;
    }

    samples_.push_back(pt);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startedAt_).count();
    sampleTimesMs_.push_back(static_cast<uint32_t>(std::max<long long>(0, elapsed)));
    lastSamplePt_ = pt;
    lastSampleAt_ = now;
}

GestureRecognizer::Result GestureRecognizer::OnButtonUp(const ScreenPoint& pt, int button) {
    if (!active_ || button != activeButton_) {
        Reset();
        return {};
    }

    OnMouseMove(pt);
    const std::vector<char> dirs = QuantizeDirections();
    Result result;
    result.gestureId = BuildGestureId(dirs);
    result.button = button;
    result.samplePoints = BuildEvaluationSamples();
    result.sampleTimesMs = BuildEvaluationSampleTimesMs();
    Reset();
    return result;
}

GestureRecognizer::Result GestureRecognizer::Snapshot() const {
    Result result;
    if (!active_) {
        return result;
    }
    result.gestureId = BuildGestureId(QuantizeDirections());
    result.button = activeButton_;
    result.samplePoints = BuildEvaluationSamples();
    result.sampleTimesMs = BuildEvaluationSampleTimesMs();
    return result;
}

bool GestureRecognizer::IsTrackedButton(int button) {
    return button == kButtonNone ||
           button == kButtonLeft ||
           button == kButtonMiddle ||
           button == kButtonRight;
}

std::vector<ScreenPoint> GestureRecognizer::BuildEvaluationSamples() const {
    std::vector<ScreenPoint> points = samples_;
    if (!active_) {
        return points;
    }
    if (points.empty()) {
        points.push_back(lastRawPt_);
        return points;
    }
    const ScreenPoint& tail = points.back();
    if (tail.x != lastRawPt_.x || tail.y != lastRawPt_.y) {
        points.push_back(lastRawPt_);
    }
    return points;
}

std::vector<uint32_t> GestureRecognizer::BuildEvaluationSampleTimesMs() const {
    std::vector<uint32_t> values = sampleTimesMs_;
    if (!active_) {
        return values;
    }
    if (values.empty()) {
        values.push_back(0);
        return values;
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(lastRawAt_ - startedAt_).count();
    const uint32_t tailMs = static_cast<uint32_t>(std::max<long long>(0, elapsed));
    if (values.size() < samples_.size()) {
        values.push_back(tailMs);
    } else if (!values.empty()) {
        values.back() = std::max(values.back(), tailMs);
    }
    return values;
}

} // namespace mousefx
