#include "pch.h"

#include "WasmPluginTransferService.Internal.h"

#include "WasmPluginCatalog.h"
#include "WasmPluginPaths.h"

#include <filesystem>
#include <set>

namespace mousefx::wasm::transfer_detail {
namespace {

constexpr const char* kExportErrorNoPluginsDiscovered = "no_plugins_discovered";
constexpr const char* kExportErrorPrimaryRootResolveFailed = "primary_root_resolve_failed";
constexpr const char* kExportErrorExportRootResolveFailed = "export_root_resolve_failed";
constexpr const char* kExportErrorCreateExportDirectoryFailed = "create_export_directory_failed";
constexpr const char* kExportErrorCopyFailed = "copy_failed";
constexpr const char* kExportErrorNoPluginCopied = "no_plugin_copied";

void SetExportError(PluginExportResult* out, const char* errorCode, const std::string& errorText) {
    if (!out) {
        return;
    }
    out->errorCode = errorCode ? errorCode : "";
    out->error = errorText;
}

} // namespace

PluginExportResult ExportAllDiscoveredPluginsImpl(const std::vector<std::wstring>& roots) {
    PluginExportResult out{};

    WasmPluginCatalog catalog;
    const PluginCatalogResult discovered = roots.empty()
        ? catalog.Discover()
        : catalog.DiscoverFromRoots(roots);
    if (discovered.plugins.empty()) {
        SetExportError(&out, kExportErrorNoPluginsDiscovered, "no plugins discovered");
        return out;
    }

    const std::filesystem::path primaryRoot(WasmPluginPaths::ResolvePrimaryPluginRoot());
    if (primaryRoot.empty()) {
        SetExportError(&out, kExportErrorPrimaryRootResolveFailed, "failed to resolve primary plugin root");
        return out;
    }

    const std::filesystem::path appRoot = primaryRoot.parent_path().parent_path();
    if (appRoot.empty()) {
        SetExportError(&out, kExportErrorExportRootResolveFailed, "failed to resolve export root");
        return out;
    }

    const std::filesystem::path exportRoot =
        appRoot / L"exports" / L"wasm" / (L"all-" + BuildTimestampLabel());
    std::error_code ec;
    std::filesystem::create_directories(exportRoot, ec);
    if (ec) {
        SetExportError(&out, kExportErrorCreateExportDirectoryFailed, "failed to create export directory");
        return out;
    }

    std::set<std::wstring> usedNames;
    uint32_t copied = 0;
    for (const DiscoveredPlugin& plugin : discovered.plugins) {
        const std::filesystem::path manifestPath(plugin.manifestPath);
        const std::filesystem::path pluginDir = manifestPath.parent_path();
        const std::filesystem::path pluginDirNormalized = NormalizePath(pluginDir);
        if (pluginDirNormalized.empty()) {
            continue;
        }

        std::wstring baseName = ResolvePluginDirectoryName(plugin.manifest, pluginDirNormalized);
        if (baseName.empty()) {
            baseName = L"plugin";
        }

        std::wstring candidate = baseName;
        int suffix = 2;
        while (usedNames.find(candidate) != usedNames.end()) {
            candidate = baseName + L"_" + std::to_wstring(suffix);
            ++suffix;
        }
        usedNames.insert(candidate);

        const std::filesystem::path destinationDir = exportRoot / candidate;
        std::string copyError;
        if (!CopyDirectoryTree(pluginDirNormalized, destinationDir, &copyError)) {
            SetExportError(&out, kExportErrorCopyFailed, copyError);
            return out;
        }
        ++copied;
    }

    out.ok = copied > 0;
    out.exportedPluginCount = copied;
    out.exportDirectoryPath = NormalizePath(exportRoot).wstring();
    if (!out.ok) {
        SetExportError(&out, kExportErrorNoPluginCopied, "no plugin copied");
    } else {
        out.error.clear();
        out.errorCode.clear();
    }
    return out;
}

} // namespace mousefx::wasm::transfer_detail
