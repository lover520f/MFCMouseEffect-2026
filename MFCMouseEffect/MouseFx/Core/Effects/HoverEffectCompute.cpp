#include "pch.h"

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>

namespace mousefx {
namespace {

bool ContainsToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

} // namespace

std::string NormalizeHoverEffectType(const std::string& effectType) {
    const std::string lowered = ToLowerAscii(effectType);
    if (lowered.empty() || lowered == "none") {
        return "glow";
    }
    if (ContainsToken(lowered, "tube") ||
        ContainsToken(lowered, "suspension") ||
        ContainsToken(lowered, "helix")) {
        return "tubes";
    }
    return "glow";
}

HoverEffectRenderCommand ComputeHoverEffectRenderCommand(
    const ScreenPoint& overlayPoint,
    const std::string& effectType,
    const HoverEffectProfile& profile) {
    HoverEffectRenderCommand command{};
    command.overlayPoint = overlayPoint;
    command.normalizedType = NormalizeHoverEffectType(effectType);
    command.tubesMode = (command.normalizedType == "tubes");

    const double sizeScale = command.tubesMode ? profile.tubesSizeScale : profile.glowSizeScale;
    const double breatheScale = command.tubesMode ? profile.tubesBreatheScale : profile.glowBreatheScale;
    const double spinScale = command.tubesMode ? profile.tubesSpinScale : 1.0;
    command.sizePx = static_cast<int>(std::lround(std::clamp(profile.sizePx * sizeScale, 96.0, 260.0)));
    command.breatheDurationSec = std::clamp(profile.breatheDurationSec * breatheScale, 0.35, 4.2);
    command.tubesSpinDurationSec = std::clamp(profile.spinDurationSec * spinScale, 0.65, 5.2);
    command.baseOpacity = std::clamp(profile.baseOpacity, 0.05, 1.0);
    command.glowFillArgb = profile.colors.glowFillArgb;
    command.glowStrokeArgb = profile.colors.glowStrokeArgb;
    command.tubesStrokeArgb = profile.colors.tubesStrokeArgb;
    return command;
}

} // namespace mousefx
