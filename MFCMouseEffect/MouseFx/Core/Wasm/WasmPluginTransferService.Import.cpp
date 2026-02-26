#include "pch.h"

#include "WasmPluginTransferService.Internal.h"

#include "WasmPluginPaths.h"

#include <filesystem>

namespace mousefx::wasm::transfer_detail {
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

void SetImportError(PluginImportResult* out, const char* errorCode, const std::string& errorText) {
    if (!out) {
        return;
    }
    out->errorCode = errorCode ? errorCode : "";
    out->error = errorText;
}

} // namespace

PluginImportResult ImportFromManifestPathImpl(const std::wstring& sourceManifestPath) {
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

} // namespace mousefx::wasm::transfer_detail
