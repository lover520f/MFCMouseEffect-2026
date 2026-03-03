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
    const std::string lowered = ToLowerAscii(TrimAscii(effectType));
    if (lowered.empty() || lowered == "none") {
        return "ripple";
    }
    if (lowered == "star" || lowered == "icon" || lowered == "icon_star") {
        return "star";
    }
    if (lowered == "text" || lowered == "textclick" || lowered == "text_click" ||
        lowered.find("text") != std::string::npos) {
        return "text";
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
    command.textFontSizePx = std::clamp(profile.textFontSizePx, 8.0, 220.0);
    command.textFloatDistancePx = std::clamp(profile.textFloatDistancePx, 0.0, 420.0);
    command.animationDurationSec =
        (command.normalizedType == "text") ? profile.textDurationSec : profile.normalDurationSec;
    command.closePaddingMs = profile.closePaddingMs;
    command.baseOpacity = std::clamp(profile.baseOpacity, 0.05, 1.0);
    if (command.normalizedType == "star") {
        command.startRadiusPx = std::clamp(profile.starStartRadiusPx, 0.0, 640.0);
        command.endRadiusPx = std::clamp(profile.starEndRadiusPx, 1.0, 800.0);
        command.strokeWidthPx = std::clamp(profile.starStrokeWidthPx, 0.1, 64.0);
    } else {
        command.startRadiusPx = std::clamp(profile.rippleStartRadiusPx, 0.0, 640.0);
        command.endRadiusPx = std::clamp(profile.rippleEndRadiusPx, 1.0, 800.0);
        command.strokeWidthPx = std::clamp(profile.rippleStrokeWidthPx, 0.1, 64.0);
    }
    command.endRadiusPx = std::max(command.endRadiusPx, command.startRadiusPx + 1.0);

    const auto& palette = ResolvePalette(button, profile);
    command.fillArgb = palette.fillArgb;
    command.strokeArgb = palette.strokeArgb;
    command.glowArgb = palette.glowArgb;
    return command;
}

} // namespace mousefx
