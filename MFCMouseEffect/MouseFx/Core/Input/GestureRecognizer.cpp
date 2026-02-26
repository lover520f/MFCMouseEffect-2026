#include "pch.h"
#include "GestureRecognizer.h"

#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <cmath>

namespace mousefx {
namespace {

constexpr int kButtonLeft = 1;
constexpr int kButtonRight = 2;
constexpr int kButtonMiddle = 3;

int ButtonFromName(const std::string& name) {
    if (name == "left") return kButtonLeft;
    if (name == "middle") return kButtonMiddle;
    return kButtonRight;
}

} // namespace

void GestureRecognizer::UpdateConfig(const GestureRecognitionConfig& config) {
    config_ = config;
    config_.triggerButton = NormalizeButtonName(config_.triggerButton);
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
    lastRawPt_ = ScreenPoint{};
    lastSamplePt_ = ScreenPoint{};
}

void GestureRecognizer::OnButtonDown(const ScreenPoint& pt, int button) {
    Reset();
    if (!config_.enabled) {
        return;
    }
    if (!IsTrackedButton(config_.triggerButton, button)) {
        return;
    }

    active_ = true;
    activeButton_ = button;
    lastRawPt_ = pt;
    lastSamplePt_ = pt;
    samples_.push_back(pt);
}

void GestureRecognizer::OnMouseMove(const ScreenPoint& pt) {
    if (!active_) {
        return;
    }

    const long long dxRaw = static_cast<long long>(pt.x) - static_cast<long long>(lastRawPt_.x);
    const long long dyRaw = static_cast<long long>(pt.y) - static_cast<long long>(lastRawPt_.y);
    totalDistancePx_ += static_cast<int>(std::sqrt(static_cast<double>(dxRaw * dxRaw + dyRaw * dyRaw)));
    lastRawPt_ = pt;

    const int stepSq = config_.sampleStepPx * config_.sampleStepPx;
    if (DistanceSquared(lastSamplePt_, pt) < stepSq) {
        return;
    }

    samples_.push_back(pt);
    lastSamplePt_ = pt;
}

std::string GestureRecognizer::OnButtonUp(const ScreenPoint& pt, int button) {
    if (!active_ || button != activeButton_) {
        Reset();
        return {};
    }

    OnMouseMove(pt);
    const std::vector<char> dirs = QuantizeDirections();
    const std::string gesture = BuildGestureId(dirs);
    Reset();
    return gesture;
}

std::string GestureRecognizer::NormalizeButtonName(std::string button) {
    button = ToLowerAscii(TrimAscii(button));
    if (button == "l" || button == "left_button") return "left";
    if (button == "m" || button == "middle_button") return "middle";
    if (button == "r" || button == "right_button") return "right";
    if (button != "left" && button != "middle" && button != "right") {
        return "right";
    }
    return button;
}

bool GestureRecognizer::IsTrackedButton(const std::string& triggerButton, int button) {
    return ButtonFromName(triggerButton) == button;
}

} // namespace mousefx
