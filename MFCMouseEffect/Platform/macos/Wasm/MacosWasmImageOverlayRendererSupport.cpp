#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"
#include "MouseFx/Utils/StringUtils.h"

#include <cmath>

namespace mousefx::platform::macos::wasm_image_overlay_support {

CGFloat ClampAlpha(float alpha) {
    return wasm_overlay_render_math::ClampFloat((alpha > 0.0f) ? static_cast<CGFloat>(alpha) : 1.0, 0.15, 1.0);
}

uint32_t ClampDelayMs(uint32_t delayMs) {
    return std::clamp<uint32_t>(delayMs, 0u, 60000u);
}

bool HasMotion(const WasmImageOverlayRequest& request) {
    return std::abs(request.velocityX) > 0.001f ||
           std::abs(request.velocityY) > 0.001f ||
           std::abs(request.accelerationX) > 0.001f ||
           std::abs(request.accelerationY) > 0.001f;
}

NSString* NsPathFromWide(const std::wstring& path) {
    if (path.empty()) {
        return nil;
    }
    const std::string utf8 = Utf16ToUtf8(path.c_str());
    if (utf8.empty()) {
        return nil;
    }
    return [NSString stringWithUTF8String:utf8.c_str()];
}

} // namespace mousefx::platform::macos::wasm_image_overlay_support
