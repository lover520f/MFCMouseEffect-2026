#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mousefx::wasm {

struct PluginImportResult final {
    bool ok = false;
    std::string error{};
    std::wstring sourceManifestPath{};
    std::wstring destinationManifestPath{};
    std::wstring primaryRootPath{};
};

struct PluginExportResult final {
    bool ok = false;
    std::string error{};
    std::wstring exportDirectoryPath{};
    uint32_t exportedPluginCount = 0;
};

class WasmPluginTransferService final {
public:
    PluginImportResult ImportFromManifestPath(const std::wstring& sourceManifestPath) const;
    PluginExportResult ExportAllDiscoveredPlugins(const std::vector<std::wstring>& roots = {}) const;
};

} // namespace mousefx::wasm
