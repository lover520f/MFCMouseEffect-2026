#pragma once

#include <string>
#include <vector>

#include "WasmPluginManifest.h"

namespace mousefx::wasm {

struct DiscoveredPlugin final {
    PluginManifest manifest{};
    std::wstring manifestPath{};
    std::wstring wasmPath{};
};

struct PluginCatalogResult final {
    std::vector<DiscoveredPlugin> plugins{};
    std::vector<std::string> errors{};
};

class WasmPluginCatalog final {
public:
    PluginCatalogResult Discover() const;
    PluginCatalogResult DiscoverFromRoots(const std::vector<std::wstring>& roots) const;
};

} // namespace mousefx::wasm

