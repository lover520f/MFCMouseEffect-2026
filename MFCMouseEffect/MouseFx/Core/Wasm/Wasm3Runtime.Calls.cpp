#include "pch.h"

#include "Wasm3Runtime.h"

#include "Wasm3Runtime.Internal.h"

#include <cstring>
#include <limits>

#include "WasmRuntimeBridge/third_party/wasm3/source/m3_env.h"
#include "WasmRuntimeBridge/third_party/wasm3/source/wasm3.h"

namespace mousefx::wasm {

bool Wasm3Runtime::CallGetApiVersion(uint32_t* outApiVersion, std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (outApiVersion) {
        *outApiVersion = 0;
    }
    if (!IsModuleLoaded()) {
        SetOutError(outError, "plugin module is not loaded.");
        return false;
    }

    M3Result result = m3_Call(fnGetApiVersion_, 0, nullptr);
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_Call(get_api_version) failed", result));
        return false;
    }

    uint32_t apiVersion = 0;
    const void* retPointers[1] = { &apiVersion };
    result = m3_GetResults(fnGetApiVersion_, 1, retPointers);
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_GetResults(get_api_version) failed", result));
        return false;
    }

    if (outApiVersion) {
        *outApiVersion = apiVersion;
    }
    SetOutError(outError, "");
    return true;
}

bool Wasm3Runtime::CallOnEvent(
    const uint8_t* inputPtr,
    uint32_t inputLen,
    uint8_t* outputPtr,
    uint32_t outputCap,
    uint32_t* outWrittenBytes,
    std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (outWrittenBytes) {
        *outWrittenBytes = 0;
    }
    if (!IsModuleLoaded()) {
        SetOutError(outError, "plugin does not export mfx_plugin_on_event.");
        return false;
    }
    if (inputLen > 0 && inputPtr == nullptr) {
        SetOutError(outError, "input pointer is null.");
        return false;
    }
    if (outputCap > 0 && outputPtr == nullptr) {
        SetOutError(outError, "output pointer is null.");
        return false;
    }

    uint32_t inputOffset = 0;
    uint32_t outputOffset = 0;
    if (!EnsureScratchMemory(inputLen, outputCap, &inputOffset, &outputOffset, outError)) {
        return false;
    }

    uint32_t memorySize = 0;
    uint8_t* memory = m3_GetMemory(runtime_, &memorySize, 0);
    if (!memory) {
        SetOutError(outError, "wasm linear memory is unavailable.");
        return false;
    }
    if (inputLen > 0) {
        std::memcpy(memory + inputOffset, inputPtr, inputLen);
    }

    const uint32_t argInputOffset = inputOffset;
    const uint32_t argInputLen = inputLen;
    const uint32_t argOutputOffset = outputOffset;
    const uint32_t argOutputCap = outputCap;
    const void* argPointers[4] = {
        &argInputOffset,
        &argInputLen,
        &argOutputOffset,
        &argOutputCap,
    };

    M3Result result = m3_Call(fnOnEvent_, 4, argPointers);
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_Call(on_event) failed", result));
        return false;
    }

    uint32_t writtenBytes = 0;
    const void* retPointers[1] = { &writtenBytes };
    result = m3_GetResults(fnOnEvent_, 1, retPointers);
    if (result) {
        SetOutError(outError, BuildWasm3Error("m3_GetResults(on_event) failed", result));
        return false;
    }
    if (writtenBytes > outputCap) {
        SetOutError(outError, "plugin returned bytes larger than output capacity.");
        return false;
    }

    if (writtenBytes > 0) {
        uint32_t memorySizeAfterCall = 0;
        uint8_t* memoryAfterCall = m3_GetMemory(runtime_, &memorySizeAfterCall, 0);
        if (!memoryAfterCall) {
            SetOutError(outError, "wasm linear memory is unavailable after plugin call.");
            return false;
        }
        const uint64_t outputEnd = static_cast<uint64_t>(outputOffset) + static_cast<uint64_t>(writtenBytes);
        if (outputEnd > static_cast<uint64_t>(memorySizeAfterCall)) {
            SetOutError(outError, "plugin output range exceeds wasm linear memory.");
            return false;
        }
        std::memcpy(outputPtr, memoryAfterCall + outputOffset, writtenBytes);
    }

    if (outWrittenBytes) {
        *outWrittenBytes = writtenBytes;
    }
    SetOutError(outError, "");
    return true;
}

void Wasm3Runtime::ResetPluginState() {
    if (!moduleLoaded_ || !runtime_ || !fnReset_) {
        return;
    }
    (void)m3_Call(fnReset_, 0, nullptr);
}

bool Wasm3Runtime::EnsureScratchMemory(
    uint32_t inputLen,
    uint32_t outputCap,
    uint32_t* outInputOffset,
    uint32_t* outOutputOffset,
    std::string* outError) {
    using namespace wasm3_runtime_detail;
    if (!runtime_ || !outInputOffset || !outOutputOffset) {
        SetOutError(outError, "runtime scratch preparation failed.");
        return false;
    }

    const uint32_t inputOffset = 0;
    const uint32_t outputOffset = AlignUp(inputLen, kScratchAlignment);
    const uint64_t requiredBytes = static_cast<uint64_t>(outputOffset) + static_cast<uint64_t>(outputCap);
    if (requiredBytes > static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)())) {
        SetOutError(outError, "scratch memory request exceeds 32-bit addressable range.");
        return false;
    }

    uint32_t currentBytes = m3_GetMemorySize(runtime_);
    if (currentBytes < requiredBytes) {
        const uint32_t requiredPages = static_cast<uint32_t>((requiredBytes + kWasmPageBytes - 1u) / kWasmPageBytes);
        M3Result resizeResult = ResizeMemory(runtime_, requiredPages);
        if (resizeResult) {
            SetOutError(outError, BuildWasm3Error("ResizeMemory failed", resizeResult));
            return false;
        }
        currentBytes = m3_GetMemorySize(runtime_);
    }

    uint32_t memorySize = 0;
    uint8_t* memory = m3_GetMemory(runtime_, &memorySize, 0);
    if (!memory) {
        SetOutError(outError, "wasm memory allocation returned null.");
        return false;
    }
    if (memorySize < requiredBytes || currentBytes < requiredBytes) {
        SetOutError(outError, "wasm memory is smaller than requested scratch range.");
        return false;
    }

    *outInputOffset = inputOffset;
    *outOutputOffset = outputOffset;
    return true;
}

} // namespace mousefx::wasm
