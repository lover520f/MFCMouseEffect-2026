#include "pch.h"

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>

namespace mousefx {
namespace {

const ClickEffectPalette& ResolvePalette(MouseButton button, const ClickEffectProfile& profile) {
    switch (button) {
    case MouseButton::Right:
        return profile.right;
    case MouseButton::Middle:
        return profile.middle;
    case MouseButton::Left:
    default:
        return profile.left;
    }
}

const char* ResolveTextLabel(MouseButton button) {
    switch (button) {
    case MouseButton::Right:
        return "RIGHT";
    case MouseButton::Middle:
        return "MIDDLE";
    case MouseButton::Left:
    default:
        return "LEFT";
    }
}

} // namespace

std::string NormalizeClickEffectType(const std::string& effectType) {
    const std::string lowered = ToLowerAscii(effectType);
    if (lowered == "star" || lowered == "text") {
        return lowered;
    }
    return "ripple";
}

ClickEffectRenderCommand ComputeClickEffectRenderCommand(
    const ScreenPoint& overlayPoint,
    MouseButton button,
    const std::string& effectType,
    const ClickEffectProfile& profile) {
    ClickEffectRenderCommand command{};
    command.overlayPoint = overlayPoint;
    command.button = button;
    command.normalizedType = NormalizeClickEffectType(effectType);
    command.textLabel = ResolveTextLabel(button);
    command.sizePx = (command.normalizedType == "text") ? profile.textSizePx : profile.normalSizePx;
    command.animationDurationSec =
        (command.normalizedType == "text") ? profile.textDurationSec : profile.normalDurationSec;
    command.closePaddingMs = profile.closePaddingMs;
    command.baseOpacity = std::clamp(profile.baseOpacity, 0.05, 1.0);

    const auto& palette = ResolvePalette(button, profile);
    command.fillArgb = palette.fillArgb;
    command.strokeArgb = palette.strokeArgb;
    command.glowArgb = palette.glowArgb;
    return command;
}

} // namespace mousefx
