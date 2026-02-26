#pragma once

#include "WasmPluginManifest.h"
#include "WasmPluginTransferService.h"

#include <filesystem>
#include <string>
#include <vector>

namespace mousefx::wasm::transfer_detail {

std::filesystem::path NormalizePath(const std::filesystem::path& path);
std::wstring ToPathKey(const std::filesystem::path& path);
std::wstring ResolvePluginDirectoryName(const PluginManifest& manifest, const std::filesystem::path& sourceDir);
bool ValidateManifestEntryFile(
    const PluginManifest& manifest,
    const std::filesystem::path& pluginDir,
    std::string* error);
bool CopyDirectoryTree(
    const std::filesystem::path& from,
    const std::filesystem::path& to,
    std::string* error);
std::wstring BuildTimestampLabel();

PluginImportResult ImportFromManifestPathImpl(const std::wstring& sourceManifestPath);
PluginExportResult ExportAllDiscoveredPluginsImpl(const std::vector<std::wstring>& roots);

} // namespace mousefx::wasm::transfer_detail
