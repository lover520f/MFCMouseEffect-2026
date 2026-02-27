#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

namespace mousefx {

struct ScrollEffectDirectionColorProfile {
    uint32_t fillArgb = 0;
    uint32_t strokeArgb = 0;
};

struct ScrollEffectProfile {
    int verticalSizePx = 138;
    int horizontalSizePx = 148;
    double baseDurationSec = 0.28;
    double perStrengthStepSec = 0.018;
    int closePaddingMs = 90;
    double baseOpacity = 0.96;
    double defaultDurationScale = 1.0;
    double helixDurationScale = 1.14;
    double twinkleDurationScale = 0.88;
    double defaultSizeScale = 1.0;
    double helixSizeScale = 1.06;
    double twinkleSizeScale = 0.94;
    ScrollEffectDirectionColorProfile horizontalPositive{};
    ScrollEffectDirectionColorProfile horizontalNegative{};
    ScrollEffectDirectionColorProfile verticalPositive{};
    ScrollEffectDirectionColorProfile verticalNegative{};
};

struct ScrollEffectRenderCommand {
    bool emit = false;
    ScreenPoint overlayPoint{};
    bool horizontal = false;
    int delta = 0;
    int strengthLevel = 0;
    std::string normalizedType = "arrow";
    bool helixMode = false;
    bool twinkleMode = false;
    int sizePx = 138;
    double durationSec = 0.28;
    int closeAfterMs = 90;
    double baseOpacity = 0.96;
    uint32_t fillArgb = 0;
    uint32_t strokeArgb = 0;
};

std::string NormalizeScrollEffectType(const std::string& effectType);
int ResolveScrollStrengthLevel(int delta);
ScrollEffectRenderCommand ComputeScrollEffectRenderCommand(
    const ScreenPoint& overlayPoint,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const ScrollEffectProfile& profile);

} // namespace mousefx
