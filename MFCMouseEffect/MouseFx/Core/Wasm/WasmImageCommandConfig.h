#pragma once

#include "MouseFx/Core/Wasm/WasmPluginAbi.h"

#include <algorithm>
#include <cmath>

namespace mousefx::wasm {

inline SpawnImageCommandV1 ResolveSpawnImageCommand(const SpawnImageCommandV1& cmd) {
    return cmd;
}

inline SpawnImageCommandV1 ResolveSpawnImageCommand(const SpawnImageAffineCommandV1& cmd) {
    SpawnImageCommandV1 resolved = cmd.base;

    // Keep existing behavior: translation offsets are always applied.
    resolved.x += cmd.affineDx;
    resolved.y += cmd.affineDy;

    // Affine matrix extras are only applied when explicitly enabled.
    if (cmd.affineEnabled != 0u) {
        const float axisXScale = std::hypot(cmd.affineM11, cmd.affineM21);
        const float axisYScale = std::hypot(cmd.affineM12, cmd.affineM22);
        if (std::isfinite(axisXScale) && std::isfinite(axisYScale)) {
            const float affineScale = std::clamp((axisXScale + axisYScale) * 0.5f, 0.2f, 5.0f);
            const float baseScale = (resolved.scale > 0.0f) ? resolved.scale : 1.0f;
            resolved.scale = baseScale * affineScale;
        }

        const float affineRotation = std::atan2(cmd.affineM21, cmd.affineM11);
        if (std::isfinite(affineRotation)) {
            resolved.rotation += affineRotation;
        }
    }

    return resolved;
}

} // namespace mousefx::wasm
