#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include <cmath>

namespace mousefx {
namespace {

constexpr int32_t kOriginGlitchRepairDistancePx = 24;
constexpr uint32_t kOriginMoveAcceptStreak = 3;

bool IsScreenOriginPoint(const ScreenPoint& pt) {
    return pt.x == 0 && pt.y == 0;
}

double PointDistance(const ScreenPoint& lhs, const ScreenPoint& rhs) {
    const double dx = static_cast<double>(lhs.x - rhs.x);
    const double dy = static_cast<double>(lhs.y - rhs.y);
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace

ScreenPoint MacosGlobalInputHook::SanitizeMouseMovePoint(const ScreenPoint& rawPt) {
    if (!IsScreenOriginPoint(rawPt)) {
        originMoveStreak_.store(0, std::memory_order_release);
        RememberStableMovePoint(rawPt);
        return rawPt;
    }

    const uint32_t streak =
        originMoveStreak_.fetch_add(1, std::memory_order_acq_rel) + 1;
    ScreenPoint stablePt{};
    if (!TryReadStableMovePoint(&stablePt)) {
        return rawPt;
    }

    if (PointDistance(stablePt, rawPt) < kOriginGlitchRepairDistancePx) {
        RememberStableMovePoint(rawPt);
        return rawPt;
    }

    if (streak >= kOriginMoveAcceptStreak) {
        RememberStableMovePoint(rawPt);
        return rawPt;
    }

    return stablePt;
}

void MacosGlobalInputHook::RememberStableMovePoint(const ScreenPoint& pt) {
    stableMoveX_.store(pt.x, std::memory_order_release);
    stableMoveY_.store(pt.y, std::memory_order_release);
    hasStableMovePoint_.store(true, std::memory_order_release);
}

bool MacosGlobalInputHook::TryReadStableMovePoint(ScreenPoint* outPt) const {
    if (!outPt) {
        return false;
    }
    if (!hasStableMovePoint_.load(std::memory_order_acquire)) {
        return false;
    }
    outPt->x = stableMoveX_.load(std::memory_order_acquire);
    outPt->y = stableMoveY_.load(std::memory_order_acquire);
    return true;
}

} // namespace mousefx
