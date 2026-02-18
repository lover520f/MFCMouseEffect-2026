#pragma once

#include <cstdint>
#include <string>

namespace mousefx::wasm {

struct PluginManifest final {
    std::string id{};
    std::string name{};
    std::string version{};
    uint32_t apiVersion = 0;
    std::wstring entryWasm{};
};

struct PluginManifestLoadResult final {
    bool ok = false;
    PluginManifest manifest{};
    std::string error{};
};

class WasmPluginManifest final {
public:
    static PluginManifestLoadResult LoadFromFile(const std::wstring& manifestPath);
    static bool Validate(const PluginManifest& manifest, std::string* outError);
};

} // namespace mousefx::wasm

