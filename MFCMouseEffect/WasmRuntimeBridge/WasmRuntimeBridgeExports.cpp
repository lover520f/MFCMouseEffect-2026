#include "WasmRuntimeBridgeContext.h"

namespace {

using wasm_runtime_bridge::RuntimeBridgeContext;

RuntimeBridgeContext* AsContext(void* handle) {
    return static_cast<RuntimeBridgeContext*>(handle);
}

constexpr const char* kNullHandleError = "runtime bridge handle is null.";

} // namespace

extern "C" {

__declspec(dllexport) void* __cdecl mfx_wasm_runtime_create() {
    try {
        auto context = wasm_runtime_bridge::CreateRuntimeBridge();
        return context.release();
    } catch (...) {
        return nullptr;
    }
}

__declspec(dllexport) void __cdecl mfx_wasm_runtime_destroy(void* handle) {
    auto* context = AsContext(handle);
    delete context;
}

__declspec(dllexport) int __cdecl mfx_wasm_runtime_load_module_file(void* handle, const wchar_t* modulePath) {
    auto* context = AsContext(handle);
    if (!context) {
        return 0;
    }
    return context->LoadModuleFromFile(modulePath) ? 1 : 0;
}

__declspec(dllexport) void __cdecl mfx_wasm_runtime_unload_module(void* handle) {
    auto* context = AsContext(handle);
    if (!context) {
        return;
    }
    context->UnloadModule();
}

__declspec(dllexport) int __cdecl mfx_wasm_runtime_is_module_loaded(void* handle) {
    auto* context = AsContext(handle);
    if (!context) {
        return 0;
    }
    return context->IsModuleLoaded() ? 1 : 0;
}

__declspec(dllexport) int __cdecl mfx_wasm_runtime_call_get_api_version(void* handle, uint32_t* outApiVersion) {
    auto* context = AsContext(handle);
    if (!context) {
        if (outApiVersion) {
            *outApiVersion = 0;
        }
        return 0;
    }
    return context->CallGetApiVersion(outApiVersion) ? 1 : 0;
}

__declspec(dllexport) int __cdecl mfx_wasm_runtime_call_on_event(
    void* handle,
    const uint8_t* inputPtr,
    uint32_t inputLen,
    uint8_t* outputPtr,
    uint32_t outputCap,
    uint32_t* outWrittenBytes) {
    auto* context = AsContext(handle);
    if (!context) {
        if (outWrittenBytes) {
            *outWrittenBytes = 0;
        }
        return 0;
    }
    return context->CallOnEvent(inputPtr, inputLen, outputPtr, outputCap, outWrittenBytes) ? 1 : 0;
}

__declspec(dllexport) void __cdecl mfx_wasm_runtime_reset_plugin(void* handle) {
    auto* context = AsContext(handle);
    if (!context) {
        return;
    }
    context->ResetPluginState();
}

__declspec(dllexport) const char* __cdecl mfx_wasm_runtime_last_error(void* handle) {
    auto* context = AsContext(handle);
    if (!context) {
        return kNullHandleError;
    }
    return context->LastError();
}

} // extern "C"
