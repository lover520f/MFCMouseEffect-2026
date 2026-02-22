#include "pch.h"

#include "Platform/PlatformWasmRuntimeFactory.h"

#include "Platform/windows/Wasm/Win32DllWasmRuntime.h"

namespace mousefx::platform {

std::unique_ptr<mousefx::wasm::IWasmRuntime> CreateDynamicBridgeWasmRuntime(std::string* outError) {
    auto runtime = std::make_unique<mousefx::wasm::Win32DllWasmRuntime>();
    if (!runtime->Initialize(outError)) {
        return nullptr;
    }
    return runtime;
}

} // namespace mousefx::platform
