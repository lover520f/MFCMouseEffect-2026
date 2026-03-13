#include "pch.h"
#include "GestureRecognizer.h"

namespace mousefx {
namespace {

int AbsInt(int v) {
    return (v < 0) ? -v : v;
}

char QuantizeDirection(const ScreenPoint& from, const ScreenPoint& to) {
    const int dx = to.x - from.x;
    int dy = to.y - from.y;
#if MFX_PLATFORM_MACOS
    // Keep gesture semantics consistent across platforms: "down" means
    // moving toward larger screen Y in a top-left-origin mental model.
    // Quartz reports Y-up coordinates, so invert dy on macOS.
    dy = -dy;
#endif
    if (dx == 0 && dy == 0) {
        return '\0';
    }
    const int absDx = AbsInt(dx);
    const int absDy = AbsInt(dy);
    const int major = (absDx > absDy) ? absDx : absDy;
    const int minor = (absDx > absDy) ? absDy : absDx;
    if (minor * 5 >= major * 2) {
        if (dx >= 0 && dy >= 0) return 'E';
        if (dx >= 0) return 'A';
        if (dy >= 0) return 'C';
        return 'Q';
    }
    if (absDx >= absDy) {
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
    case 'Q': return "diag_up_left";
    case 'A': return "diag_up_right";
    case 'C': return "diag_down_left";
    case 'E': return "diag_down_right";
    default: return "";
    }
}

bool IsCardinalDirection(char dir) {
    return dir == 'L' || dir == 'R' || dir == 'U' || dir == 'D';
}

bool IsDiagonalDirection(char dir) {
    return dir == 'Q' || dir == 'A' || dir == 'C' || dir == 'E';
}

int HorizontalSign(char dir) {
    switch (dir) {
    case 'L':
    case 'Q':
    case 'C':
        return -1;
    case 'R':
    case 'A':
    case 'E':
        return 1;
    default:
        return 0;
    }
}

int VerticalSign(char dir) {
    switch (dir) {
    case 'U':
    case 'Q':
    case 'A':
        return -1;
    case 'D':
    case 'C':
    case 'E':
        return 1;
    default:
        return 0;
    }
}

void SimplifyDirectionChain(std::vector<char>* dirs) {
    if (!dirs) {
        return;
    }
    bool changed = true;
    while (changed && dirs->size() >= 3) {
        changed = false;
        for (size_t i = 1; i + 1 < dirs->size(); ++i) {
            const char prev = (*dirs)[i - 1];
            const char mid = (*dirs)[i];
            const char next = (*dirs)[i + 1];
            // Remove cardinal jitter between same diagonals/axes: A-B-A -> A.
            if (prev == next && IsCardinalDirection(mid)) {
                dirs->erase(dirs->begin() + static_cast<std::ptrdiff_t>(i),
                    dirs->begin() + static_cast<std::ptrdiff_t>(i + 2));
                changed = true;
                break;
            }
            // Remove short cardinal jitter between diagonals that keep the same
            // major intent axis, e.g. E-R-A => E-A (common W/V micro-jitter).
            if (IsDiagonalDirection(prev) && IsDiagonalDirection(next) && IsCardinalDirection(mid)) {
                const int prevHx = HorizontalSign(prev);
                const int nextHx = HorizontalSign(next);
                const int prevVy = VerticalSign(prev);
                const int nextVy = VerticalSign(next);
                const int midHx = HorizontalSign(mid);
                const int midVy = VerticalSign(mid);
                const bool horizontalBridge =
                    (midHx != 0 && prevHx == nextHx && prevHx == midHx);
                const bool verticalBridge =
                    (midVy != 0 && prevVy == nextVy && prevVy == midVy);
                if (horizontalBridge || verticalBridge) {
                    dirs->erase(dirs->begin() + static_cast<std::ptrdiff_t>(i));
                    changed = true;
                    break;
                }
            }
        }
    }
}

} // namespace

std::vector<char> GestureRecognizer::QuantizeDirections() const {
    std::vector<char> dirs;
    const std::vector<ScreenPoint> points = BuildEvaluationSamples();
    if (totalDistancePx_ < config_.minStrokeDistancePx || points.size() < 2) {
        return dirs;
    }

    const int stepSq = config_.sampleStepPx * config_.sampleStepPx;
    char prev = '\0';
    for (size_t i = 1; i < points.size(); ++i) {
        if (DistanceSquared(points[i - 1], points[i]) < stepSq) {
            continue;
        }
        const char dir = QuantizeDirection(points[i - 1], points[i]);
        if (dir == '\0' || dir == prev) {
            continue;
        }
        dirs.push_back(dir);
        prev = dir;
        if (dirs.size() >= 64) {
            break;
        }
    }
    SimplifyDirectionChain(&dirs);
    if (static_cast<int>(dirs.size()) > config_.maxDirections) {
        // Prefer keeping the latest simplified segments so late-stage intent is
        // not lost by early jitter when user finishes the gesture.
        dirs.erase(
            dirs.begin(),
            dirs.end() - static_cast<std::ptrdiff_t>(config_.maxDirections));
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
