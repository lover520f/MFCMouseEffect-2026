#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Wasm/WasmPluginAbi.h"

#include <algorithm>
#include <cmath>

namespace mousefx::wasm {

inline TextConfig BuildSpawnTextConfig(const TextConfig& baseConfig, const SpawnTextCommandV1& cmd) {
    TextConfig cfg = baseConfig;
    if (cmd.lifeMs > 0) {
        cfg.durationMs = std::clamp<int>(static_cast<int>(cmd.lifeMs), 80, 8000);
    }

    const float lifeSeconds = std::max(0.08f, static_cast<float>(cfg.durationMs) / 1000.0f);
    const float predictedDy = (cmd.vy * lifeSeconds) + (0.5f * cmd.ay * lifeSeconds * lifeSeconds);
    const float fallbackDy = std::abs(cmd.vy) * 0.55f;
    const float distance = std::max(std::abs(predictedDy), fallbackDy);
    cfg.floatDistance = std::clamp<int>(static_cast<int>(std::lround(distance)), 16, 420);

    if (cmd.scale > 0.0f) {
        const float scaledSize = cfg.fontSize * cmd.scale;
        cfg.fontSize = std::clamp(scaledSize, 6.0f, 90.0f);
    }
    return cfg;
}

} // namespace mousefx::wasm
