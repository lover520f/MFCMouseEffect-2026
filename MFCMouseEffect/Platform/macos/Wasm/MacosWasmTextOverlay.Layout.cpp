#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmTextOverlay.Internal.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

namespace mousefx::platform::macos {

#if defined(__APPLE__)
WasmTextOverlayLayout BuildWasmTextOverlayLayout(
    const ScreenPoint& screenPt,
    size_t utf8Length,
    float scale,
    uint32_t lifeMs) {
    const ScreenPoint pt = ScreenToOverlayPoint(screenPt);

    WasmTextOverlayLayout layout{};
    layout.durationMs = wasm_overlay_render_math::ClampLifeMs(lifeMs);
    const CGFloat textScale = wasm_overlay_render_math::ClampScale(scale);
    layout.fontSize = wasm_overlay_render_math::ClampFloat(13.0 * textScale, 9.0, 42.0);
    layout.width = wasm_overlay_render_math::ClampFloat(
        18.0 + static_cast<CGFloat>(utf8Length) * (layout.fontSize * 0.72),
        64.0,
        460.0);
    layout.height = wasm_overlay_render_math::ClampFloat(layout.fontSize * 2.0, 30.0, 88.0);
    layout.frame = NSMakeRect(pt.x - layout.width * 0.5, pt.y - layout.height - 24.0, layout.width, layout.height);
    return layout;
}

#endif

} // namespace mousefx::platform::macos
