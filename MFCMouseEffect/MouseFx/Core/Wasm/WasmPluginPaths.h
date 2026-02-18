#pragma once

#include <string>
#include <vector>

#include "WasmPluginManifest.h"

namespace mousefx::wasm {

class WasmPluginPaths final {
public:
    static std::wstring ResolvePrimaryPluginRoot();
    static std::vector<std::wstring> ResolveSearchRoots();
    static std::wstring ResolveEntryWasmPath(const std::wstring& manifestPath, const PluginManifest& manifest);
};

} // namespace mousefx::wasm

