#include "pch.h"

#include "WasmRuntimeFactory.h"

#include "DllWasmRuntime.h"
#include "NullWasmRuntime.h"

namespace mousefx::wasm {

std::unique_ptr<IWasmRuntime> CreateRuntime(RuntimeBackend backend) {
    switch (backend) {
    case RuntimeBackend::DynamicBridge: {
        auto runtime = std::make_unique<DllWasmRuntime>();
        std::string error;
        if (runtime->Initialize(&error)) {
            return runtime;
        }
        return std::make_unique<NullWasmRuntime>();
    }
    case RuntimeBackend::Null:
    default:
        return std::make_unique<NullWasmRuntime>();
    }
}

std::unique_ptr<IWasmRuntime> CreateDefaultRuntime() {
    return CreateRuntime(RuntimeBackend::DynamicBridge);
}

} // namespace mousefx::wasm
