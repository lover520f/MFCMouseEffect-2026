#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"

namespace mousefx::platform::macos::wasm_image_overlay_core_detail {

ImageOverlayRenderPlan BuildImageOverlayRenderPlan(const WasmImageOverlayRequest& request) {
    ImageOverlayRenderPlan plan{};
    const ScreenPoint overlayPoint = ScreenToOverlayPoint(request.screenPt);
    const CGFloat pulseScale = wasm_overlay_render_math::ClampScale(request.scale);
    plan.overlayPoint = overlayPoint;
    plan.size = wasm_overlay_render_math::ClampFloat(120.0 * pulseScale, 52.0, 420.0);
    plan.durationMs = wasm_overlay_render_math::ClampLifeMs(request.lifeMs);
    plan.delayMs = wasm_image_overlay_support::ClampDelayMs(request.delayMs);
    plan.alphaScale = wasm_image_overlay_support::ClampAlpha(request.alpha);
    plan.request = request;
    return plan;
}

} // namespace mousefx::platform::macos::wasm_image_overlay_core_detail
