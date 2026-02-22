#include "pch.h"

#include "Platform/windows/Wasm/Win32DllWasmRuntime.h"

#include "MouseFx/Utils/StringUtils.h"

#include <array>
#include <filesystem>

namespace mousefx::wasm {

namespace {

std::filesystem::path ModuleDirectory() {
    std::array<wchar_t, 4096> buffer{};
    const DWORD n = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (n == 0 || n >= buffer.size()) {
        return {};
    }
    return std::filesystem::path(buffer.data()).parent_path();
}

} // namespace

Win32DllWasmRuntime::~Win32DllWasmRuntime() {
    Shutdown();
}

bool Win32DllWasmRuntime::Initialize(std::string* outError) {
    if (module_ && runtimeHandle_) {
        if (outError) {
            outError->clear();
        }
        return true;
    }

    const std::filesystem::path exeDir = ModuleDirectory();
    const std::filesystem::path candidate = exeDir / L"mfx_wasm_runtime.dll";
    module_ = LoadLibraryW(candidate.wstring().c_str());
    if (!module_) {
        module_ = LoadLibraryW(L"mfx_wasm_runtime.dll");
    }
    if (!module_) {
        if (outError) {
            *outError = "Cannot load mfx_wasm_runtime.dll.";
        }
        return false;
    }
    if (!ResolveExports(outError)) {
        Shutdown();
        return false;
    }

    runtimeHandle_ = createFn_ ? createFn_() : nullptr;
    if (!runtimeHandle_) {
        if (outError) {
            *outError = "Runtime bridge create() failed.";
        }
        Shutdown();
        return false;
    }
    if (outError) {
        outError->clear();
    }
    return true;
}

bool Win32DllWasmRuntime::LoadModuleFromFile(const std::wstring& modulePath, std::string* outError) {
    if (!loadModuleFn_ || !runtimeHandle_) {
        if (outError) {
            *outError = "Runtime bridge is not initialized.";
        }
        return false;
    }
    const int ok = loadModuleFn_(runtimeHandle_, modulePath.c_str());
    if (!ok) {
        if (outError) {
            *outError = ReadBridgeError();
        }
        return false;
    }
    if (outError) {
        outError->clear();
    }
    return true;
}

void Win32DllWasmRuntime::UnloadModule() {
    if (unloadModuleFn_ && runtimeHandle_) {
        unloadModuleFn_(runtimeHandle_);
    }
}

bool Win32DllWasmRuntime::IsModuleLoaded() const {
    if (!isModuleLoadedFn_ || !runtimeHandle_) {
        return false;
    }
    return isModuleLoadedFn_(runtimeHandle_) != 0;
}

bool Win32DllWasmRuntime::CallGetApiVersion(uint32_t* outApiVersion, std::string* outError) {
    if (outApiVersion) {
        *outApiVersion = 0;
    }
    if (!getApiVersionFn_ || !runtimeHandle_) {
        if (outError) {
            *outError = "Runtime bridge is not initialized.";
        }
        return false;
    }
    uint32_t version = 0;
    const int ok = getApiVersionFn_(runtimeHandle_, &version);
    if (!ok) {
        if (outError) {
            *outError = ReadBridgeError();
        }
        return false;
    }
    if (outApiVersion) {
        *outApiVersion = version;
    }
    if (outError) {
        outError->clear();
    }
    return true;
}

bool Win32DllWasmRuntime::CallOnEvent(
    const uint8_t* inputPtr,
    uint32_t inputLen,
    uint8_t* outputPtr,
    uint32_t outputCap,
    uint32_t* outWrittenBytes,
    std::string* outError) {
    if (outWrittenBytes) {
        *outWrittenBytes = 0;
    }
    if (!onEventFn_ || !runtimeHandle_) {
        if (outError) {
            *outError = "Runtime bridge does not support mfx_plugin_on_event.";
        }
        return false;
    }
    uint32_t written = 0;
    const int ok = onEventFn_(runtimeHandle_, inputPtr, inputLen, outputPtr, outputCap, &written);
    if (!ok) {
        if (outError) {
            *outError = ReadBridgeError();
        }
        return false;
    }
    if (outWrittenBytes) {
        *outWrittenBytes = written;
    }
    if (outError) {
        outError->clear();
    }
    return true;
}

void Win32DllWasmRuntime::ResetPluginState() {
    if (resetFn_ && runtimeHandle_) {
        resetFn_(runtimeHandle_);
    }
}

bool Win32DllWasmRuntime::ResolveExports(std::string* outError) {
    struct ExportRoute {
        const char* name;
        void** fnPtr;
    };
    const ExportRoute routes[] = {
        {"mfx_wasm_runtime_create", reinterpret_cast<void**>(&createFn_)},
        {"mfx_wasm_runtime_destroy", reinterpret_cast<void**>(&destroyFn_)},
        {"mfx_wasm_runtime_load_module_file", reinterpret_cast<void**>(&loadModuleFn_)},
        {"mfx_wasm_runtime_unload_module", reinterpret_cast<void**>(&unloadModuleFn_)},
        {"mfx_wasm_runtime_is_module_loaded", reinterpret_cast<void**>(&isModuleLoadedFn_)},
        {"mfx_wasm_runtime_call_get_api_version", reinterpret_cast<void**>(&getApiVersionFn_)},
        {"mfx_wasm_runtime_call_on_event", reinterpret_cast<void**>(&onEventFn_)},
        {"mfx_wasm_runtime_reset_plugin", reinterpret_cast<void**>(&resetFn_)},
        {"mfx_wasm_runtime_last_error", reinterpret_cast<void**>(&lastErrorFn_)},
    };

    for (const ExportRoute& route : routes) {
        FARPROC proc = GetProcAddress(module_, route.name);
        if (!proc) {
            if (outError) {
                *outError = std::string("Missing runtime export: ") + route.name;
            }
            return false;
        }
        *(route.fnPtr) = reinterpret_cast<void*>(proc);
    }

    if (outError) {
        outError->clear();
    }
    return true;
}

std::string Win32DllWasmRuntime::ReadBridgeError() const {
    if (!lastErrorFn_ || !runtimeHandle_) {
        return "Runtime bridge call failed.";
    }
    const char* message = lastErrorFn_(runtimeHandle_);
    return message ? EnsureUtf8(message) : "Runtime bridge call failed.";
}

void Win32DllWasmRuntime::Shutdown() {
    if (destroyFn_ && runtimeHandle_) {
        destroyFn_(runtimeHandle_);
    }
    runtimeHandle_ = nullptr;
    createFn_ = nullptr;
    destroyFn_ = nullptr;
    loadModuleFn_ = nullptr;
    unloadModuleFn_ = nullptr;
    isModuleLoadedFn_ = nullptr;
    getApiVersionFn_ = nullptr;
    onEventFn_ = nullptr;
    resetFn_ = nullptr;
    lastErrorFn_ = nullptr;
    if (module_) {
        FreeLibrary(module_);
        module_ = nullptr;
    }
}

} // namespace mousefx::wasm
