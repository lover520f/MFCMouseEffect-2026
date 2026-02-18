#include "pch.h"

#include "WasmPluginCatalog.h"

#include "WasmPluginPaths.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <filesystem>
#include <set>
#include <sstream>
#include <system_error>
#include <cwchar>

namespace mousefx::wasm {

namespace {

std::vector<std::filesystem::path> FindManifestFiles(const std::wstring& root) {
    std::vector<std::filesystem::path> files;
    if (root.empty()) {
        return files;
    }

    std::error_code ec;
    const std::filesystem::path rootPath(root);
    if (!std::filesystem::exists(rootPath, ec) || ec) {
        return files;
    }

    std::filesystem::recursive_directory_iterator it(
        rootPath,
        std::filesystem::directory_options::skip_permission_denied,
        ec);
    const std::filesystem::recursive_directory_iterator end;
    while (!ec && it != end) {
        const auto& entry = *it;
        std::error_code localEc;
        if (entry.is_regular_file(localEc) && !localEc) {
            const std::wstring filename = entry.path().filename().wstring();
            if (_wcsicmp(filename.c_str(), L"plugin.json") == 0) {
                files.push_back(entry.path());
            }
        }
        it.increment(ec);
        if (ec) {
            ec.clear();
        }
    }
    return files;
}

void AppendCatalogError(
    const std::filesystem::path& manifestPath,
    const std::string& error,
    std::vector<std::string>* errors) {
    if (!errors) {
        return;
    }
    std::ostringstream ss;
    ss << "manifest=" << Utf16ToUtf8(manifestPath.wstring().c_str()) << " error=" << error;
    errors->push_back(ss.str());
}

} // namespace

PluginCatalogResult WasmPluginCatalog::Discover() const {
    return DiscoverFromRoots(WasmPluginPaths::ResolveSearchRoots());
}

PluginCatalogResult WasmPluginCatalog::DiscoverFromRoots(const std::vector<std::wstring>& roots) const {
    PluginCatalogResult result{};
    std::set<std::string> seenPluginIds;

    for (const std::wstring& root : roots) {
        const std::vector<std::filesystem::path> manifests = FindManifestFiles(root);
        for (const std::filesystem::path& manifestPath : manifests) {
            const PluginManifestLoadResult load = WasmPluginManifest::LoadFromFile(manifestPath.wstring());
            if (!load.ok) {
                AppendCatalogError(manifestPath, load.error, &result.errors);
                continue;
            }
            if (seenPluginIds.find(load.manifest.id) != seenPluginIds.end()) {
                AppendCatalogError(manifestPath, "Duplicated plugin id.", &result.errors);
                continue;
            }

            const std::wstring wasmPath = WasmPluginPaths::ResolveEntryWasmPath(manifestPath.wstring(), load.manifest);
            if (wasmPath.empty()) {
                AppendCatalogError(manifestPath, "Cannot resolve wasm entry path.", &result.errors);
                continue;
            }
            std::error_code ec;
            if (!std::filesystem::exists(std::filesystem::path(wasmPath), ec) || ec) {
                AppendCatalogError(manifestPath, "WASM entry file does not exist.", &result.errors);
                continue;
            }

            DiscoveredPlugin plugin{};
            plugin.manifest = load.manifest;
            plugin.manifestPath = manifestPath.wstring();
            plugin.wasmPath = wasmPath;
            result.plugins.push_back(std::move(plugin));
            seenPluginIds.insert(load.manifest.id);
        }
    }

    std::sort(result.plugins.begin(), result.plugins.end(), [](const DiscoveredPlugin& lhs, const DiscoveredPlugin& rhs) {
        return ToLowerAscii(lhs.manifest.id) < ToLowerAscii(rhs.manifest.id);
    });
    return result;
}

} // namespace mousefx::wasm
