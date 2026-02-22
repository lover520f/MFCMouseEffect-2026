#include "pch.h"

#include "NullWasmRuntime.h"

namespace mousefx::wasm {

namespace {

void SetBackendMissingError(std::string* outError) {
    if (outError) {
        *outError = "WASM runtime backend is not linked.";
    }
}

} // namespace

bool NullWasmRuntime::LoadModuleFromFile(const std::wstring& modulePath, std::string* outError) {
    lastModulePath_ = modulePath;
    SetBackendMissingError(outError);
    return false;
}

void NullWasmRuntime::UnloadModule() {
    lastModulePath_.clear();
}

bool NullWasmRuntime::IsModuleLoaded() const {
    return false;
}

bool NullWasmRuntime::CallGetApiVersion(uint32_t* outApiVersion, std::string* outError) {
    if (outApiVersion) {
        *outApiVersion = 0;
    }
    SetBackendMissingError(outError);
    return false;
}

bool NullWasmRuntime::CallOnEvent(
    const uint8_t* inputPtr,
    uint32_t inputLen,
    uint8_t* outputPtr,
    uint32_t outputCap,
    uint32_t* outWrittenBytes,
    std::string* outError) {
    (void)inputPtr;
    (void)inputLen;
    (void)outputPtr;
    (void)outputCap;
    if (outWrittenBytes) {
        *outWrittenBytes = 0;
    }
    SetBackendMissingError(outError);
    return false;
}

void NullWasmRuntime::ResetPluginState() {
    // Intentionally no-op for placeholder runtime.
}

} // namespace mousefx::wasm

