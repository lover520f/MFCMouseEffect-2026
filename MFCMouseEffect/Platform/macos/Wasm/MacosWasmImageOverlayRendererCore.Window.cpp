#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.h"
#include "Platform/macos/Wasm/MacosWasmImageOverlaySwiftBridge.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#endif

#include <cmath>
#include <memory>

namespace mousefx::platform::macos {

#if defined(__APPLE__)
namespace {

struct WasmImageOverlayCloseContext final {
    void* windowHandle = nullptr;
};

void CloseWasmImageOverlayAfterDelay(void* context) {
    std::unique_ptr<WasmImageOverlayCloseContext> closeContext(
        static_cast<WasmImageOverlayCloseContext*>(context));
    if (!closeContext || closeContext->windowHandle == nullptr) {
        return;
    }
    if (!TakeWasmOverlayWindow(closeContext->windowHandle)) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(closeContext->windowHandle);
}

} // namespace
#endif

void RenderWasmImageOverlayWindowOnMain(
    const wasm_image_overlay_core_detail::ImageOverlayRenderPlan& plan) {
#if !defined(__APPLE__)
    (void)plan;
    return;
#else
    const WasmImageOverlayRequest req = plan.request;
    const double frameX = plan.overlayPoint.x - plan.size * 0.5;
    const double frameY = plan.overlayPoint.y - plan.size * 0.5;
    const double t = static_cast<double>(plan.durationMs) / 1000.0;
    const double motionDx = wasm_image_overlay_support::HasMotion(req)
        ? (req.velocityX * t) + (0.5 * req.accelerationX * t * t)
        : 0.0;
    const double motionDy = wasm_image_overlay_support::HasMotion(req)
        ? (req.velocityY * t) + (0.5 * req.accelerationY * t * t)
        : 0.0;
    const std::string imagePathUtf8 = req.assetPath.empty()
        ? std::string{}
        : wasm_image_overlay_support::Utf8PathFromWide(req.assetPath);

    void* windowHandle = mfx_macos_wasm_image_overlay_create_v1(
        frameX,
        frameY,
        plan.size,
        imagePathUtf8.empty() ? nullptr : imagePathUtf8.c_str(),
        req.tintArgb,
        req.applyTint ? 1 : 0,
        plan.alphaScale,
        t,
        req.rotationRad,
        motionDx,
        motionDy);
    if (windowHandle == nullptr) {
        ReleaseWasmOverlaySlot();
        return;
    }
    RegisterWasmOverlayWindow(windowHandle);
    mfx_macos_wasm_image_overlay_show_v1(windowHandle);

    auto* closeContext = new WasmImageOverlayCloseContext{};
    closeContext->windowHandle = windowHandle;
    dispatch_after_f(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.durationMs + 60u) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        closeContext,
        &CloseWasmImageOverlayAfterDelay);
#endif
}

} // namespace mousefx::platform::macos
