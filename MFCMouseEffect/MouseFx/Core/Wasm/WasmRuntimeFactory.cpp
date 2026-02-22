#include "pch.h"

#include "WasmRuntimeFactory.h"

#include "NullWasmRuntime.h"
#include "Platform/PlatformWasmRuntimeFactory.h"

namespace mousefx::wasm {

const char* RuntimeBackendToString(RuntimeBackend backend) {
    switch (backend) {
    case RuntimeBackend::DynamicBridge:
        return "dynamic_bridge";
    case RuntimeBackend::Null:
    default:
        return "null";
    }
}

std::unique_ptr<IWasmRuntime> CreateRuntime(RuntimeBackend backend) {
    switch (backend) {
    case RuntimeBackend::DynamicBridge: {
        std::string error;
        auto runtime = platform::CreateDynamicBridgeWasmRuntime(&error);
        if (runtime) {
            return runtime;
        }
        return std::make_unique<NullWasmRuntime>();
    }
    case RuntimeBackend::Null:
    default:
        return std::make_unique<NullWasmRuntime>();
    }
}

RuntimeCreationResult CreateDefaultRuntimeWithDiagnostics() {
    RuntimeCreationResult result{};

    std::string error;
    auto runtime = platform::CreateDynamicBridgeWasmRuntime(&error);
    if (runtime) {
        result.runtime = std::move(runtime);
        result.backend = RuntimeBackend::DynamicBridge;
        return result;
    }

    result.runtime = std::make_unique<NullWasmRuntime>();
    result.backend = RuntimeBackend::Null;
    result.fallbackReason = "dynamic bridge unavailable";
    if (!error.empty()) {
        result.fallbackReason += ": " + error;
    }
    return result;
}

std::unique_ptr<IWasmRuntime> CreateDefaultRuntime() {
    RuntimeCreationResult result = CreateDefaultRuntimeWithDiagnostics();
    return std::move(result.runtime);
}

} // namespace mousefx::wasm
