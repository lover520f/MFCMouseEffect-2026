#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

namespace mousefx::platform::macos::wasm_image_overlay_core_detail {

ImageOverlayRenderPlan BuildImageOverlayRenderPlan(const WasmImageOverlayRequest& request) {
    ImageOverlayRenderPlan plan{};
    const ScreenPoint overlayPoint = ScreenToOverlayPoint(request.screenPt);
    const CGFloat pulseScale = wasm_overlay_render_math::ClampScale(request.scale);
    plan.overlayPoint = overlayPoint;
    plan.size = wasm_overlay_render_math::ClampFloat(120.0 * pulseScale, 52.0, 420.0);
    plan.durationMs = wasm_overlay_render_math::ClampLifeMs(request.lifeMs);
    plan.delayMs = request.delayMs;
    plan.alphaScale = wasm_overlay_render_math::ClampFloat(static_cast<CGFloat>(request.alpha), 0.0, 1.0);
    plan.request = request;
    return plan;
}

} // namespace mousefx::platform::macos::wasm_image_overlay_core_detail
