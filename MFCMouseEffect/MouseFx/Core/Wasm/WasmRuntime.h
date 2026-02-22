#pragma once

#include <cstdint>
#include <string>

namespace mousefx::wasm {

class IWasmRuntime {
public:
    virtual ~IWasmRuntime() = default;

    virtual bool LoadModuleFromFile(const std::wstring& modulePath, std::string* outError) = 0;
    virtual void UnloadModule() = 0;
    virtual bool IsModuleLoaded() const = 0;

    virtual bool CallGetApiVersion(uint32_t* outApiVersion, std::string* outError) = 0;
    virtual bool CallOnEvent(
        const uint8_t* inputPtr,
        uint32_t inputLen,
        uint8_t* outputPtr,
        uint32_t outputCap,
        uint32_t* outWrittenBytes,
        std::string* outError) = 0;

    virtual void ResetPluginState() = 0;
};

} // namespace mousefx::wasm

