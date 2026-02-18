#pragma once

#include "WasmRuntime.h"

namespace mousefx::wasm {

// Runtime placeholder used before integrating a concrete WASM backend.
class NullWasmRuntime final : public IWasmRuntime {
public:
    bool LoadModuleFromFile(const std::wstring& modulePath, std::string* outError) override;
    void UnloadModule() override;
    bool IsModuleLoaded() const override;

    bool CallGetApiVersion(uint32_t* outApiVersion, std::string* outError) override;
    bool CallOnClick(
        const uint8_t* inputPtr,
        uint32_t inputLen,
        uint8_t* outputPtr,
        uint32_t outputCap,
        uint32_t* outWrittenBytes,
        std::string* outError) override;

    void ResetPluginState() override;

private:
    std::wstring lastModulePath_{};
};

} // namespace mousefx::wasm

