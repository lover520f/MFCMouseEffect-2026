#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "WasmRuntime.h"

namespace mousefx::wasm {

enum class RuntimeBackend : uint8_t {
    Null = 0,
    DynamicBridge = 1,
};

const char* RuntimeBackendToString(RuntimeBackend backend);

struct RuntimeCreationResult final {
    std::unique_ptr<IWasmRuntime> runtime{};
    RuntimeBackend backend = RuntimeBackend::Null;
    std::string fallbackReason{};
};

std::unique_ptr<IWasmRuntime> CreateRuntime(RuntimeBackend backend);
RuntimeCreationResult CreateDefaultRuntimeWithDiagnostics();
std::unique_ptr<IWasmRuntime> CreateDefaultRuntime();

} // namespace mousefx::wasm
