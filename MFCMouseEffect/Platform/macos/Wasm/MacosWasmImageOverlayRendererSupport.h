#pragma once

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.h"

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

#include <cstdint>
#include <string>

namespace mousefx::platform::macos::wasm_image_overlay_support {

CGFloat ClampAlpha(float alpha);
uint32_t ClampDelayMs(uint32_t delayMs);
bool HasMotion(const WasmImageOverlayRequest& request);
std::string Utf8PathFromWide(const std::wstring& path);

} // namespace mousefx::platform::macos::wasm_image_overlay_support
