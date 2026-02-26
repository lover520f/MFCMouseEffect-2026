#include "pch.h"
#include "GestureRecognizer.h"

namespace mousefx {
namespace {

int AbsInt(int v) {
    return (v < 0) ? -v : v;
}

char QuantizeDirection(const ScreenPoint& from, const ScreenPoint& to) {
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

long long GestureRecognizer::DistanceSquared(const ScreenPoint& a, const ScreenPoint& b) const {
    const long long dx = static_cast<long long>(b.x) - static_cast<long long>(a.x);
    const long long dy = static_cast<long long>(b.y) - static_cast<long long>(a.y);
    return dx * dx + dy * dy;
}

} // namespace mousefx
