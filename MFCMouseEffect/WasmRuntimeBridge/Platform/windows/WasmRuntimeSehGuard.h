#pragma once

#include <cstdint>

#include "third_party/wasm3/source/wasm3.h"

namespace wasm_runtime_bridge {

enum class ProtectedInvokeStatus : uint8_t {
    Ok = 0,
    SehFault,
};

ProtectedInvokeStatus SafeM3Call(
    IM3Function function,
    uint32_t argc,
    const void* args[],
    M3Result* outResult,
    uint32_t* outSehCode) noexcept;

ProtectedInvokeStatus SafeM3GetResults(
    IM3Function function,
    uint32_t retc,
    const void* retPointers[],
    M3Result* outResult,
    uint32_t* outSehCode) noexcept;

} // namespace wasm_runtime_bridge
