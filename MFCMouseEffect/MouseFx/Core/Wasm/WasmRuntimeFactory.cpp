#include "pch.h"

#include "WasmRuntimeFactory.h"

#include "NullWasmRuntime.h"

namespace mousefx::wasm {

std::unique_ptr<IWasmRuntime> CreateRuntime(RuntimeBackend backend) {
    switch (backend) {
    case RuntimeBackend::Null:
    default:
        return std::make_unique<NullWasmRuntime>();
    }
}

} // namespace mousefx::wasm

