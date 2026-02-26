#include "pch.h"

#include "WasmPluginTransferService.h"

#include "WasmPluginTransferService.Internal.h"

namespace mousefx::wasm {

PluginImportResult WasmPluginTransferService::ImportFromManifestPath(const std::wstring& sourceManifestPath) const {
    return transfer_detail::ImportFromManifestPathImpl(sourceManifestPath);
}

PluginExportResult WasmPluginTransferService::ExportAllDiscoveredPlugins(const std::vector<std::wstring>& roots) const {
    return transfer_detail::ExportAllDiscoveredPluginsImpl(roots);
}

} // namespace mousefx::wasm
