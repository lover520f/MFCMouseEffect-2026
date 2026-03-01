#pragma once

#include <algorithm>
#include <cstdint>

namespace mousefx::wasm {

inline float ResolveSpawnImageScale(float scale) {
    return (scale > 0.0f) ? scale : 1.0f;
}

inline float ResolveSpawnImageAlpha(float alpha) {
    return (alpha > 0.0f) ? std::clamp(alpha, 0.0f, 1.0f) : 1.0f;
}

inline uint32_t ResolveSpawnImageDelayMs(uint32_t delayMs) {
    return std::clamp<uint32_t>(delayMs, 0u, 60000u);
}

inline uint32_t ResolveSpawnImageLifeMs(uint32_t lifeMs, int configDurationMs) {
    if (lifeMs > 0u) {
        return std::clamp<uint32_t>(lifeMs, 60u, 10000u);
    }
    const int fallback = std::max(60, configDurationMs);
    return std::clamp<uint32_t>(static_cast<uint32_t>(fallback), 60u, 10000u);
}

inline bool ResolveSpawnImageApplyTint(uint32_t tintArgb) {
    return ((tintArgb >> 24) & 0xFFu) != 0u;
}

} // namespace mousefx::wasm
