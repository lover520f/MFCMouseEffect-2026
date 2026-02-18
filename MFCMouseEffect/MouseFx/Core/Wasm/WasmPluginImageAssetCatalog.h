#pragma once

#include <cstdint>
#include <string>

namespace mousefx::wasm {

class WasmPluginImageAssetCatalog final {
public:
    static bool ResolveImageAssetPath(
        const std::wstring& manifestPath,
        uint32_t imageId,
        std::wstring* outImagePath,
        std::string* outError);
};

} // namespace mousefx::wasm

