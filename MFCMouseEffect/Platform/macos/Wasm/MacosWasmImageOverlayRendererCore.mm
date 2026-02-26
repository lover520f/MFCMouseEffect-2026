#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h"
#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#endif

namespace mousefx::platform::macos {

WasmOverlayRenderResult RenderWasmImageOverlayCore(const WasmImageOverlayRequest& request) {
#if !defined(__APPLE__)
    (void)request;
    return WasmOverlayRenderResult::Failed;
#else
    const WasmOverlayAdmissionResult admission = TryAcquireWasmOverlaySlot(WasmOverlayKind::Image);
    if (admission != WasmOverlayAdmissionResult::Accepted) {
        return (admission == WasmOverlayAdmissionResult::RejectedByCapacity)
            ? WasmOverlayRenderResult::ThrottledByCapacity
            : WasmOverlayRenderResult::ThrottledByInterval;
    }

    const wasm_image_overlay_core_detail::ImageOverlayRenderPlan plan =
        wasm_image_overlay_core_detail::BuildImageOverlayRenderPlan(request);

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.delayMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          RenderWasmImageOverlayWindowOnMain(plan);
        });

    return WasmOverlayRenderResult::Rendered;
#endif
}

} // namespace mousefx::platform::macos
