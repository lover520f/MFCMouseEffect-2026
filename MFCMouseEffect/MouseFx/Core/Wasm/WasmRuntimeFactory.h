#pragma once

#include <cstdint>
#include <memory>

#include "WasmRuntime.h"

namespace mousefx::wasm {

enum class RuntimeBackend : uint8_t {
    Null = 0,
};

std::unique_ptr<IWasmRuntime> CreateRuntime(RuntimeBackend backend);

} // namespace mousefx::wasm
