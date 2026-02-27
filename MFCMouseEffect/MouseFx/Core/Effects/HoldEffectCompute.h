#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

#include <cstdint>
#include <string>

namespace mousefx {

enum class HoldEffectFollowMode {
    Precise = 0,
    Smooth = 1,
    Efficient = 2,
};

struct HoldEffectColorProfile {
    uint32_t leftBaseStrokeArgb = 0;
    uint32_t rightBaseStrokeArgb = 0;
    uint32_t middleBaseStrokeArgb = 0;
    uint32_t lightningStrokeArgb = 0;
    uint32_t hexStrokeArgb = 0;
    uint32_t hologramStrokeArgb = 0;
    uint32_t quantumHaloStrokeArgb = 0;
    uint32_t fluxFieldStrokeArgb = 0;
    uint32_t techNeonStrokeArgb = 0;
};

struct HoldEffectProfile {
    int sizePx = 188;
    int progressFullMs = 1400;
    double breatheDurationSec = 0.9;
    double rotateDurationSec = 2.2;
    double rotateDurationFastSec = 1.5;
    double baseOpacity = 0.92;
    HoldEffectColorProfile colors{};
};

struct HoldEffectStartCommand {
    ScreenPoint overlayPoint{};
    MouseButton button = MouseButton::Left;
    std::string normalizedType = "charge";
    int sizePx = 188;
    int progressFullMs = 1400;
    double breatheDurationSec = 0.9;
    double rotateDurationSec = 2.2;
    double rotateDurationFastSec = 1.5;
    double baseOpacity = 0.92;
    HoldEffectColorProfile colors{};
};

struct HoldEffectFollowState {
    bool hasSmoothedPoint = false;
    float smoothedX = 0.0f;
    float smoothedY = 0.0f;
    uint64_t lastEfficientTickMs = 0;
};

struct HoldEffectUpdateCommand {
    bool emit = false;
    ScreenPoint overlayPoint{};
    uint32_t holdMs = 0;
};

std::string NormalizeHoldEffectType(const std::string& effectType);
HoldEffectFollowMode ParseHoldEffectFollowMode(const std::string& mode);
HoldEffectStartCommand ComputeHoldEffectStartCommand(
    const ScreenPoint& overlayPoint,
    MouseButton button,
    const std::string& effectType,
    const HoldEffectProfile& profile);
HoldEffectUpdateCommand ComputeHoldEffectUpdateCommand(
    const ScreenPoint& point,
    uint32_t holdMs,
    uint64_t nowMs,
    HoldEffectFollowMode mode,
    HoldEffectFollowState* state);

} // namespace mousefx
