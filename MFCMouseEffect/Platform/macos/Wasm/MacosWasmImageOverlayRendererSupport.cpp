#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.h"

#include "MouseFx/Utils/StringUtils.h"

#include <cmath>

namespace mousefx::platform::macos::wasm_image_overlay_support {

bool HasMotion(const WasmImageOverlayRequest& request) {
    return std::abs(request.velocityX) > 0.001f ||
           std::abs(request.velocityY) > 0.001f ||
           std::abs(request.accelerationX) > 0.001f ||
           std::abs(request.accelerationY) > 0.001f;
}

std::string Utf8PathFromWide(const std::wstring& path) {
    if (path.empty()) {
        return {};
    }
    const std::string utf8 = Utf16ToUtf8(path.c_str());
    return utf8;
}

} // namespace mousefx::platform::macos::wasm_image_overlay_support
