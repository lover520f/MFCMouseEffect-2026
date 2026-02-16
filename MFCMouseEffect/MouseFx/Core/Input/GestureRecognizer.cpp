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

int AbsInt(int v) {
    return (v < 0) ? -v : v;
}

int ButtonFromName(const std::string& name) {
    if (name == "left") return kButtonLeft;
    if (name == "middle") return kButtonMiddle;
    return kButtonRight;
}

char QuantizeDirection(const POINT& from, const POINT& to) {
    const int dx = to.x - from.x;
    const int dy = to.y - from.y;
    if (dx == 0 && dy == 0) {
        return '\0';
    }
    if (AbsInt(dx) >= AbsInt(dy)) {
        return (dx >= 0) ? 'R' : 'L';
    }
    return (dy >= 0) ? 'D' : 'U';
}

const char* DirectionWord(char dir) {
    switch (dir) {
    case 'L': return "left";
    case 'R': return "right";
    case 'U': return "up";
    case 'D': return "down";
    default: return "";
    }
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
    lastRawPt_ = POINT{};
    lastSamplePt_ = POINT{};
}

void GestureRecognizer::OnButtonDown(const POINT& pt, int button) {
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

void GestureRecognizer::OnMouseMove(const POINT& pt) {
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

std::string GestureRecognizer::OnButtonUp(const POINT& pt, int button) {
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

std::vector<char> GestureRecognizer::QuantizeDirections() const {
    std::vector<char> dirs;
    if (totalDistancePx_ < config_.minStrokeDistancePx || samples_.size() < 2) {
        return dirs;
    }

    const int stepSq = config_.sampleStepPx * config_.sampleStepPx;
    char prev = '\0';
    for (size_t i = 1; i < samples_.size(); ++i) {
        if (DistanceSquared(samples_[i - 1], samples_[i]) < stepSq) {
            continue;
        }
        const char dir = QuantizeDirection(samples_[i - 1], samples_[i]);
        if (dir == '\0' || dir == prev) {
            continue;
        }
        dirs.push_back(dir);
        prev = dir;
        if (static_cast<int>(dirs.size()) >= config_.maxDirections) {
            break;
        }
    }
    return dirs;
}

std::string GestureRecognizer::BuildGestureId(const std::vector<char>& dirs) {
    if (dirs.empty()) {
        return {};
    }

    std::string out;
    for (size_t i = 0; i < dirs.size(); ++i) {
        if (i > 0) {
            out += "_";
        }
        out += DirectionWord(dirs[i]);
    }
    return out;
}

long long GestureRecognizer::DistanceSquared(const POINT& a, const POINT& b) const {
    const long long dx = static_cast<long long>(b.x) - static_cast<long long>(a.x);
    const long long dy = static_cast<long long>(b.y) - static_cast<long long>(a.y);
    return dx * dx + dy * dy;
}

} // namespace mousefx
