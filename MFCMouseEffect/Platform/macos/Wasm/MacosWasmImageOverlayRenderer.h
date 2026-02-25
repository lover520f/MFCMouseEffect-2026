#pragma once

#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"

namespace mousefx::platform::macos {

WasmOverlayRenderResult RenderWasmImageOverlay(const WasmImageOverlayRequest& request);

} // namespace mousefx::platform::macos
