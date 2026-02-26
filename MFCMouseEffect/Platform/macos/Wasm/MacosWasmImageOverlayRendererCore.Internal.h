#pragma once

#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"

namespace mousefx::platform::macos::wasm_image_overlay_core_detail {

struct ImageOverlayRenderPlan final {
    ScreenPoint overlayPoint{};
    double size = 120.0;
    uint32_t durationMs = 900;
    uint32_t delayMs = 0;
    double alphaScale = 1.0;
    WasmImageOverlayRequest request{};
};

ImageOverlayRenderPlan BuildImageOverlayRenderPlan(const WasmImageOverlayRequest& request);

} // namespace mousefx::platform::macos::wasm_image_overlay_core_detail

namespace mousefx::platform::macos {

void RenderWasmImageOverlayWindowOnMain(
    const wasm_image_overlay_core_detail::ImageOverlayRenderPlan& plan);

} // namespace mousefx::platform::macos
