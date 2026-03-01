#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

#include <algorithm>

namespace mousefx::platform::macos::wasm_overlay_render_math {

CGFloat ClampFloat(CGFloat value, CGFloat lo, CGFloat hi) {
    return std::max(lo, std::min(value, hi));
}

CGFloat ClampScale(float scale) {
    const CGFloat raw = (scale > 0.0f) ? static_cast<CGFloat>(scale) : 1.0;
    return ClampFloat(raw, 0.25, 6.0);
}

uint32_t ClampLifeMs(uint32_t lifeMs) {
    if (lifeMs == 0u) {
        return 350u;
    }
    return std::clamp<uint32_t>(lifeMs, 60u, 10000u);
}

} // namespace mousefx::platform::macos::wasm_overlay_render_math
