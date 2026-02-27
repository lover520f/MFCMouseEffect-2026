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
    double normalDurationSec = 0.32;
    double textDurationSec = 0.36;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
    ClickEffectPalette left{};
    ClickEffectPalette right{};
    ClickEffectPalette middle{};
};

struct ClickEffectRenderCommand {
    ScreenPoint overlayPoint{};
    MouseButton button = MouseButton::Left;
    std::string normalizedType = "ripple";
    std::string textLabel{};
    int sizePx = 138;
    double animationDurationSec = 0.32;
    int closePaddingMs = 60;
    double baseOpacity = 0.95;
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
