#pragma once

#include <cstdint>
#include <memory>

#include "WasmRuntime.h"

namespace mousefx::wasm {

enum class RuntimeBackend : uint8_t {
    Null = 0,
    DynamicBridge = 1,
};

std::unique_ptr<IWasmRuntime> CreateRuntime(RuntimeBackend backend);
std::unique_ptr<IWasmRuntime> CreateDefaultRuntime();

} // namespace mousefx::wasm
