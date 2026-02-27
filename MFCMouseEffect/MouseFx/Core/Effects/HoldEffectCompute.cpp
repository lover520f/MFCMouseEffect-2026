#include "pch.h"

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Effects/HoldRouteCatalog.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cmath>

namespace mousefx {
namespace {

bool ContainsToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

} // namespace

std::string NormalizeHoldEffectType(const std::string& effectType) {
    const std::string canonical = hold_route::NormalizeHoldEffectTypeAlias(effectType);
    const std::string lowered = ToLowerAscii(canonical);
    if (lowered.empty() || lowered == "none") {
        return "charge";
    }
    return lowered;
}

HoldEffectFollowMode ParseHoldEffectFollowMode(const std::string& mode) {
    const std::string lowered = ToLowerAscii(mode);
    if (lowered == "precise") {
        return HoldEffectFollowMode::Precise;
    }
    if (lowered == "efficient") {
        return HoldEffectFollowMode::Efficient;
    }
    return HoldEffectFollowMode::Smooth;
}

HoldEffectStartCommand ComputeHoldEffectStartCommand(
    const ScreenPoint& overlayPoint,
    MouseButton button,
    const std::string& effectType,
    const HoldEffectProfile& profile) {
    HoldEffectStartCommand command{};
    command.overlayPoint = overlayPoint;
    command.button = button;
    command.normalizedType = NormalizeHoldEffectType(effectType);
    command.sizePx = profile.sizePx;
    command.progressFullMs = profile.progressFullMs;
    command.breatheDurationSec = profile.breatheDurationSec;
    command.rotateDurationSec = profile.rotateDurationSec;
    command.rotateDurationFastSec = profile.rotateDurationFastSec;
    command.baseOpacity = std::clamp(profile.baseOpacity, 0.05, 1.0);
    command.colors = profile.colors;
    return command;
}

HoldEffectUpdateCommand ComputeHoldEffectUpdateCommand(
    const ScreenPoint& point,
    uint32_t holdMs,
    uint64_t nowMs,
    HoldEffectFollowMode mode,
    HoldEffectFollowState* state) {
    HoldEffectUpdateCommand command{};
    command.holdMs = holdMs;
    if (state == nullptr) {
        command.emit = true;
        command.overlayPoint = point;
        return command;
    }

    ScreenPoint output = point;
    switch (mode) {
    case HoldEffectFollowMode::Precise:
        break;
    case HoldEffectFollowMode::Smooth:
        if (!state->hasSmoothedPoint) {
            state->smoothedX = static_cast<float>(point.x);
            state->smoothedY = static_cast<float>(point.y);
            state->hasSmoothedPoint = true;
        } else {
            constexpr float kAlpha = 0.35f;
            state->smoothedX += (static_cast<float>(point.x) - state->smoothedX) * kAlpha;
            state->smoothedY += (static_cast<float>(point.y) - state->smoothedY) * kAlpha;
        }
        output.x = static_cast<int32_t>(std::lround(state->smoothedX));
        output.y = static_cast<int32_t>(std::lround(state->smoothedY));
        break;
    case HoldEffectFollowMode::Efficient:
        if (state->lastEfficientTickMs != 0 && (nowMs - state->lastEfficientTickMs) < 20) {
            return command;
        }
        state->lastEfficientTickMs = nowMs;
        break;
    }

    command.emit = true;
    command.overlayPoint = output;
    return command;
}

} // namespace mousefx
