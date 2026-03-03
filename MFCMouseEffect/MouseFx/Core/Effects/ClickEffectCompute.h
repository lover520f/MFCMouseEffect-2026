#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

namespace mousefx {

struct ClickEffectPalette {
    uint32_t fillArgb = 0;
    uint32_t strokeArgb = 0;
    uint32_t glowArgb = 0;
};

struct ClickEffectProfile {
    int normalSizePx = 138;
    int textSizePx = 152;
    double textFontSizePx = 24.0;
    double textFloatDistancePx = 60.0;
    double normalDurationSec = 0.32;
    double textDurationSec = 0.36;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
    // Geometric parameters shared by platform renderers.
    double rippleStartRadiusPx = 0.0;
    double rippleEndRadiusPx = 40.0;
    double rippleStrokeWidthPx = 2.5;
    double starStartRadiusPx = 5.0;
    double starEndRadiusPx = 35.0;
    double starStrokeWidthPx = 2.0;
    ClickEffectPalette left{};
    ClickEffectPalette right{};
    ClickEffectPalette middle{};
};

struct ClickEffectRenderCommand {
    ScreenPoint overlayPoint{};
    MouseButton button = MouseButton::Left;
    std::string normalizedType = "ripple";
    std::string textLabel{};
    std::string textFontFamilyUtf8{};
    bool textEmoji = false;
    int sizePx = 138;
    double textFontSizePx = 24.0;
    double textFloatDistancePx = 60.0;
    double animationDurationSec = 0.32;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
    double startRadiusPx = 0.0;
    double endRadiusPx = 40.0;
    double strokeWidthPx = 2.5;
    uint32_t fillArgb = 0;
    uint32_t strokeArgb = 0;
    uint32_t glowArgb = 0;
};

std::string NormalizeClickEffectType(const std::string& effectType);
ClickEffectRenderCommand ComputeClickEffectRenderCommand(
    const ScreenPoint& overlayPoint,
    MouseButton button,
    const std::string& effectType,
    const ClickEffectProfile& profile);

} // namespace mousefx
