#pragma once

#include <windows.h>

#include "MouseFx/Core/Wasm/WasmRuntime.h"

namespace mousefx::wasm {

class Win32DllWasmRuntime final : public IWasmRuntime {
public:
    Win32DllWasmRuntime() = default;
    ~Win32DllWasmRuntime() override;

    bool Initialize(std::string* outError);

    bool LoadModuleFromFile(const std::wstring& modulePath, std::string* outError) override;
    void UnloadModule() override;
    bool IsModuleLoaded() const override;

    bool CallGetApiVersion(uint32_t* outApiVersion, std::string* outError) override;
    bool CallOnEvent(
        const uint8_t* inputPtr,
        uint32_t inputLen,
        uint8_t* outputPtr,
        uint32_t outputCap,
        uint32_t* outWrittenBytes,
        std::string* outError) override;

    void ResetPluginState() override;

private:
    using CreateFn = void* (__cdecl*)();
    using DestroyFn = void(__cdecl*)(void*);
    using LoadModuleFn = int(__cdecl*)(void*, const wchar_t*);
    using UnloadModuleFn = void(__cdecl*)(void*);
    using IsModuleLoadedFn = int(__cdecl*)(void*);
    using GetApiVersionFn = int(__cdecl*)(void*, uint32_t*);
    using OnEventFn = int(__cdecl*)(void*, const uint8_t*, uint32_t, uint8_t*, uint32_t, uint32_t*);
    using ResetFn = void(__cdecl*)(void*);
    using LastErrorFn = const char* (__cdecl*)(void*);

    bool ResolveExports(std::string* outError);
    std::string ReadBridgeError() const;
    void Shutdown();

    HMODULE module_ = nullptr;
    void* runtimeHandle_ = nullptr;
    CreateFn createFn_ = nullptr;
    DestroyFn destroyFn_ = nullptr;
    LoadModuleFn loadModuleFn_ = nullptr;
    UnloadModuleFn unloadModuleFn_ = nullptr;
    IsModuleLoadedFn isModuleLoadedFn_ = nullptr;
    GetApiVersionFn getApiVersionFn_ = nullptr;
    OnEventFn onEventFn_ = nullptr;
    ResetFn resetFn_ = nullptr;
    LastErrorFn lastErrorFn_ = nullptr;
};

} // namespace mousefx::wasm

