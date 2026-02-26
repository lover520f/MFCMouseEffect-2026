#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

#include <algorithm>

namespace mousefx::platform::macos::wasm_overlay_render_math {

namespace {

uint8_t ArgbA(uint32_t argb) {
    return static_cast<uint8_t>((argb >> 24) & 0xFFu);
}

uint8_t ArgbR(uint32_t argb) {
    return static_cast<uint8_t>((argb >> 16) & 0xFFu);
}

uint8_t ArgbG(uint32_t argb) {
    return static_cast<uint8_t>((argb >> 8) & 0xFFu);
}

uint8_t ArgbB(uint32_t argb) {
    return static_cast<uint8_t>(argb & 0xFFu);
}

} // namespace

CGFloat ClampFloat(CGFloat value, CGFloat lo, CGFloat hi) {
    return std::max(lo, std::min(value, hi));
}

CGFloat ClampScale(float scale) {
    const CGFloat raw = (scale > 0.0f) ? static_cast<CGFloat>(scale) : 1.0;
    return ClampFloat(raw, 0.25, 6.0);
}

uint32_t ClampLifeMs(uint32_t lifeMs) {
    if (lifeMs == 0u) {
        return 300u;
    }
    return std::clamp<uint32_t>(lifeMs, 80u, 6000u);
}

NSColor* ColorFromArgb(uint32_t argb, CGFloat alphaScale) {
    const CGFloat a = ClampFloat((static_cast<CGFloat>(ArgbA(argb)) / 255.0) * alphaScale, 0.0, 1.0);
    const CGFloat r = static_cast<CGFloat>(ArgbR(argb)) / 255.0;
    const CGFloat g = static_cast<CGFloat>(ArgbG(argb)) / 255.0;
    const CGFloat b = static_cast<CGFloat>(ArgbB(argb)) / 255.0;
    return [NSColor colorWithCalibratedRed:r green:g blue:b alpha:a];
}

} // namespace mousefx::platform::macos::wasm_overlay_render_math
