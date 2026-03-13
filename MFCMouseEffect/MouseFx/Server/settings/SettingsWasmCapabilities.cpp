#include "pch.h"

#include "MouseFx/Server/settings/SettingsWasmCapabilities.h"

#include "MouseFx/Core/Wasm/WasmCommandRenderer.h"
#include "Platform/PlatformTarget.h"

namespace mousefx {

bool IsWasmInvokeSupportedOnCurrentPlatform() {
#if MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS
    return true;
#else
    return false;
#endif
}

bool IsWasmRenderSupportedOnCurrentPlatform() {
    static const bool supported = [] {
        auto renderer = wasm::CreatePlatformWasmCommandRenderer();
        return renderer && renderer->SupportsRendering();
    }();
    return supported;
}

} // namespace mousefx
