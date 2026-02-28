#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRenderer.h"
#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.h"

namespace mousefx::platform::macos {

WasmOverlayRenderResult RenderWasmImageOverlay(const WasmImageOverlayRequest& request) {
    return RenderWasmImageOverlayCore(request);
}

} // namespace mousefx::platform::macos
