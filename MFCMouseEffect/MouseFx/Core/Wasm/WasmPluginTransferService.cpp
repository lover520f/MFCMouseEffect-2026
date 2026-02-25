#include "pch.h"

#include "WasmPluginTransferService.h"

#include "WasmPluginCatalog.h"
#include "WasmPluginManifest.h"
#include "WasmPluginPaths.h"

#include "MouseFx/Utils/StringUtils.h"

#include <chrono>
#include <array>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <set>
#include <system_error>

namespace mousefx::wasm {

namespace {

constexpr const char* kImportErrorManifestPathRequired = "manifest_path_required";
constexpr const char* kImportErrorManifestPathNotFound = "manifest_path_not_found";
constexpr const char* kImportErrorManifestPathNotFile = "manifest_path_not_file";
constexpr const char* kImportErrorManifestLoadFailed = "manifest_load_failed";
constexpr const char* kImportErrorSourceEntryInvalid = "source_entry_invalid";
constexpr const char* kImportErrorPrimaryRootResolveFailed = "primary_root_resolve_failed";
constexpr const char* kImportErrorCopyFailed = "copy_failed";
constexpr const char* kImportErrorDestinationManifestMissing = "destination_manifest_missing";
constexpr const char* kImportErrorDestinationEntryInvalid = "destination_entry_invalid";

constexpr const char* kExportErrorNoPluginsDiscovered = "no_plugins_discovered";
constexpr const char* kExportErrorPrimaryRootResolveFailed = "primary_root_resolve_failed";
constexpr const char* kExportErrorExportRootResolveFailed = "export_root_resolve_failed";
constexpr const char* kExportErrorCreateExportDirectoryFailed = "create_export_directory_failed";
constexpr const char* kExportErrorCopyFailed = "copy_failed";
constexpr const char* kExportErrorNoPluginCopied = "no_plugin_copied";

void SetImportError(PluginImportResult* out, const char* errorCode, const std::string& errorText) {
    if (!out) {
        return;
    }
    out->errorCode = errorCode ? errorCode : "";
    out->error = errorText;
}

void SetExportError(PluginExportResult* out, const char* errorCode, const std::string& errorText) {
    if (!out) {
        return;
    }
    out->errorCode = errorCode ? errorCode : "";
    out->error = errorText;
}

std::filesystem::path NormalizePath(const std::filesystem::path& path) {
    std::error_code ec;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
    if (!ec) {
        return canonical.lexically_normal();
    }
    return path.lexically_normal();
}

std::wstring ToPathKey(const std::filesystem::path& path) {
    std::wstring key = NormalizePath(path).wstring();
    for (wchar_t& ch : key) {
        ch = static_cast<wchar_t>(::towlower(ch));
    }
    return key;
}

std::wstring SanitizeDirectoryName(const std::wstring& input) {
    if (input.empty()) {
        return {};
    }

    std::wstring out;
    out.reserve(input.size());
    for (const wchar_t ch : input) {
        if (ch == L'<' ||
            ch == L'>' ||
            ch == L':' ||
            ch == L'"' ||
            ch == L'/' ||
            ch == L'\\' ||
            ch == L'|' ||
            ch == L'?' ||
            ch == L'*' ||
            ch < 32) {
            out.push_back(L'_');
            continue;
        }
        out.push_back(ch);
    }

    while (!out.empty() && (out.back() == L' ' || out.back() == L'.')) {
        out.pop_back();
    }
    return out;
}

std::wstring ResolvePluginDirectoryName(const PluginManifest& manifest, const std::filesystem::path& sourceDir) {
    std::wstring name = SanitizeDirectoryName(Utf8ToWString(manifest.id));
    if (!name.empty()) {
        return name;
    }
    name = SanitizeDirectoryName(sourceDir.filename().wstring());
    if (!name.empty()) {
        return name;
    }
    return L"wasm_plugin";
}

bool ValidateManifestEntryFile(
    const PluginManifest& manifest,
    const std::filesystem::path& pluginDir,
    std::string* error) {
    if (pluginDir.empty()) {
        if (error) {
            *error = "plugin directory is empty";
        }
        return false;
    }

    const std::filesystem::path entryRelativePath(manifest.entryWasm);
    const std::filesystem::path entryPath = pluginDir / entryRelativePath;
    std::error_code ec;
    if (!std::filesystem::exists(entryPath, ec) || ec) {
        if (error) {
            *error = "manifest entry wasm file does not exist";
        }
        return false;
    }
    if (!std::filesystem::is_regular_file(entryPath, ec) || ec) {
        if (error) {
            *error = "manifest entry wasm path is not a file";
        }
        return false;
    }
    return true;
}

bool CopyDirectoryTree(
    const std::filesystem::path& from,
    const std::filesystem::path& to,
    std::string* error) {
    if (from.empty() || to.empty()) {
        if (error) {
            *error = "source or destination path is empty";
        }
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::exists(from, ec) || ec) {
        if (error) {
            *error = "source directory does not exist";
        }
        return false;
    }
    if (!std::filesystem::is_directory(from, ec) || ec) {
        if (error) {
            *error = "source path is not a directory";
        }
        return false;
    }

    std::filesystem::create_directories(to.parent_path(), ec);
    if (ec) {
        if (error) {
            *error = "failed to create destination parent directory";
        }
        return false;
    }

    std::filesystem::remove_all(to, ec);
    if (ec) {
        if (error) {
            *error = "failed to clear destination directory";
        }
        return false;
    }

    std::filesystem::create_directories(to, ec);
    if (ec) {
        if (error) {
            *error = "failed to create destination directory";
        }
        return false;
    }

    std::filesystem::copy(
        from,
        to,
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing,
        ec);
    if (ec) {
        if (error) {
            *error = "failed to copy plugin directory";
        }
        return false;
    }
    return true;
}

std::wstring BuildTimestampLabel() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t timeValue = std::chrono::system_clock::to_time_t(now);

    std::tm localTm{};
#if defined(_WIN32)
    if (::localtime_s(&localTm, &timeValue) != 0) {
        return L"unknown-time";
    }
#else
    if (::localtime_r(&timeValue, &localTm) == nullptr) {
        return L"unknown-time";
    }
#endif

    std::array<wchar_t, 48> buffer{};
    const int n = swprintf(
        buffer.data(),
        buffer.size(),
        L"%04u%02u%02u-%02u%02u%02u",
        static_cast<unsigned>(localTm.tm_year + 1900),
        static_cast<unsigned>(localTm.tm_mon + 1),
        static_cast<unsigned>(localTm.tm_mday),
        static_cast<unsigned>(localTm.tm_hour),
        static_cast<unsigned>(localTm.tm_min),
        static_cast<unsigned>(localTm.tm_sec));
    if (n <= 0) {
        return L"unknown-time";
    }
    return std::wstring(buffer.data(), static_cast<size_t>(n));
}

} // namespace

PluginImportResult WasmPluginTransferService::ImportFromManifestPath(const std::wstring& sourceManifestPath) const {
    PluginImportResult out{};
    out.sourceManifestPath = sourceManifestPath;

    std::wstring manifestText = sourceManifestPath;
    while (!manifestText.empty() && iswspace(manifestText.front())) {
        manifestText.erase(manifestText.begin());
    }
    while (!manifestText.empty() && iswspace(manifestText.back())) {
        manifestText.pop_back();
    }
    if (manifestText.empty()) {
        SetImportError(&out, kImportErrorManifestPathRequired, "manifest_path is required");
        return out;
    }

    const std::filesystem::path sourceManifestPathObj(manifestText);
    std::error_code ec;
    if (!std::filesystem::exists(sourceManifestPathObj, ec) || ec) {
        SetImportError(&out, kImportErrorManifestPathNotFound, "manifest_path does not exist");
        return out;
    }
    if (!std::filesystem::is_regular_file(sourceManifestPathObj, ec) || ec) {
        SetImportError(&out, kImportErrorManifestPathNotFile, "manifest_path is not a file");
        return out;
    }

    const PluginManifestLoadResult load = WasmPluginManifest::LoadFromFile(sourceManifestPathObj.wstring());
    if (!load.ok) {
        SetImportError(&out, kImportErrorManifestLoadFailed, load.error);
        return out;
    }

    const std::filesystem::path sourceDir = sourceManifestPathObj.parent_path();
    std::string entryValidationError;
    if (!ValidateManifestEntryFile(load.manifest, sourceDir, &entryValidationError)) {
        SetImportError(&out, kImportErrorSourceEntryInvalid, entryValidationError);
        return out;
    }

    const std::filesystem::path primaryRoot(WasmPluginPaths::ResolvePrimaryPluginRoot());
    if (primaryRoot.empty()) {
        SetImportError(&out, kImportErrorPrimaryRootResolveFailed, "failed to resolve primary plugin root");
        return out;
    }

    const std::wstring pluginFolderName = ResolvePluginDirectoryName(load.manifest, sourceDir);
    const std::filesystem::path destinationDir = primaryRoot / pluginFolderName;
    const std::filesystem::path sourceDirNormalized = NormalizePath(sourceDir);
    const std::filesystem::path destinationDirNormalized = NormalizePath(destinationDir);

    if (ToPathKey(sourceDirNormalized) != ToPathKey(destinationDirNormalized)) {
        std::string copyError;
        if (!CopyDirectoryTree(sourceDirNormalized, destinationDirNormalized, &copyError)) {
            SetImportError(&out, kImportErrorCopyFailed, copyError);
            return out;
        }
    }

    const std::filesystem::path destinationManifestPathObj = destinationDirNormalized / L"plugin.json";
    if (!std::filesystem::exists(destinationManifestPathObj, ec) || ec) {
        SetImportError(&out, kImportErrorDestinationManifestMissing, "destination plugin.json is missing");
        return out;
    }

    std::string destinationEntryValidationError;
    if (!ValidateManifestEntryFile(load.manifest, destinationDirNormalized, &destinationEntryValidationError)) {
        SetImportError(&out, kImportErrorDestinationEntryInvalid, destinationEntryValidationError);
        return out;
    }

    out.ok = true;
    out.error.clear();
    out.errorCode.clear();
    out.destinationManifestPath = destinationManifestPathObj.wstring();
    out.primaryRootPath = NormalizePath(primaryRoot).wstring();
    return out;
}

PluginExportResult WasmPluginTransferService::ExportAllDiscoveredPlugins(const std::vector<std::wstring>& roots) const {
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

} // namespace mousefx::wasm
