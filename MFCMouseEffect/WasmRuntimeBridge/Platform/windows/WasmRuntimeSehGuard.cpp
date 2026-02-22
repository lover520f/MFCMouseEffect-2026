#include "Platform/windows/WasmRuntimeSehGuard.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <excpt.h>
#include <windows.h>

namespace wasm_runtime_bridge {

ProtectedInvokeStatus SafeM3Call(
    IM3Function function,
    uint32_t argc,
    const void* args[],
    M3Result* outResult,
    uint32_t* outSehCode) noexcept {
    if (outResult) {
        *outResult = m3Err_none;
    }
    if (outSehCode) {
        *outSehCode = 0;
    }
    __try {
        if (outResult) {
            *outResult = m3_Call(function, argc, args);
        } else {
            (void)m3_Call(function, argc, args);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (outSehCode) {
            *outSehCode = static_cast<uint32_t>(GetExceptionCode());
        }
        return ProtectedInvokeStatus::SehFault;
    }
    return ProtectedInvokeStatus::Ok;
}

ProtectedInvokeStatus SafeM3GetResults(
    IM3Function function,
    uint32_t retc,
    const void* retPointers[],
    M3Result* outResult,
    uint32_t* outSehCode) noexcept {
    if (outResult) {
        *outResult = m3Err_none;
    }
    if (outSehCode) {
        *outSehCode = 0;
    }
    __try {
        if (outResult) {
            *outResult = m3_GetResults(function, retc, retPointers);
        } else {
            (void)m3_GetResults(function, retc, retPointers);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (outSehCode) {
            *outSehCode = static_cast<uint32_t>(GetExceptionCode());
        }
        return ProtectedInvokeStatus::SehFault;
    }
    return ProtectedInvokeStatus::Ok;
}

} // namespace wasm_runtime_bridge
