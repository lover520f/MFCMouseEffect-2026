#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRenderer.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

namespace mousefx::platform::macos {

WasmOverlayRenderResult ShowWasmImageOverlay(const WasmImageOverlayRequest& request) {
    return RenderWasmImageOverlay(request);
}

WasmOverlayRenderResult ShowWasmImagePulseOverlay(
    const ScreenPoint& screenPt,
    uint32_t tintArgb,
    float scale,
    float alpha,
    uint32_t lifeMs) {
    WasmImageOverlayRequest request{};
    request.screenPt = screenPt;
    request.tintArgb = tintArgb;
    request.scale = scale;
    request.alpha = alpha;
    request.lifeMs = lifeMs;
    return ShowWasmImageOverlay(request);
}

void CloseAllWasmOverlays() {
#if !defined(__APPLE__)
    return;
#else
    CloseAllWasmOverlayWindows();
#endif
}

} // namespace mousefx::platform::macos
