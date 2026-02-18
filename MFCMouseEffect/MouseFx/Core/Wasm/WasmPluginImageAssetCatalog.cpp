#include "pch.h"

#include "WasmPluginImageAssetCatalog.h"

#include "WasmPluginManifest.h"
#include "WasmPluginPaths.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <mutex>
#include <unordered_map>

namespace mousefx::wasm {

namespace {

struct CacheEntry final {
    bool loaded = false;
    std::filesystem::file_time_type lastWriteTime{};
    std::vector<std::wstring> imagePaths{};
    std::string error{};
};

std::wstring BuildCacheKey(const std::wstring& path) {
    std::wstring key = std::filesystem::path(path).lexically_normal().wstring();
    std::transform(key.begin(), key.end(), key.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(::towlower(ch));
    });
    return key;
}

std::filesystem::file_time_type ReadWriteTime(const std::filesystem::path& path) {
    std::error_code ec;
    const auto writeTime = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return {};
    }
    return writeTime;
}

bool LoadAssetsFromManifest(
    const std::wstring& manifestPath,
    CacheEntry* outEntry) {
    if (!outEntry) {
        return false;
    }

    outEntry->imagePaths.clear();
    outEntry->error.clear();
    outEntry->loaded = true;

    const PluginManifestLoadResult load = WasmPluginManifest::LoadFromFile(manifestPath);
    if (!load.ok) {
        outEntry->error = load.error.empty() ? "cannot load plugin manifest" : load.error;
        return false;
    }

    for (const std::wstring& relativePath : load.manifest.imageAssets) {
        const std::wstring absPath =
            WasmPluginPaths::ResolveManifestRelativePath(manifestPath, relativePath);
        if (absPath.empty()) {
            continue;
        }
        std::error_code ec;
        if (!std::filesystem::exists(std::filesystem::path(absPath), ec) || ec) {
            continue;
        }
        outEntry->imagePaths.push_back(absPath);
    }

    if (outEntry->imagePaths.empty()) {
        outEntry->error = "manifest has no valid image_assets";
        return false;
    }
    return true;
}

std::unordered_map<std::wstring, CacheEntry>& CatalogCache() {
    static std::unordered_map<std::wstring, CacheEntry> cache;
    return cache;
}

std::mutex& CatalogMutex() {
    static std::mutex lock;
    return lock;
}

} // namespace

bool WasmPluginImageAssetCatalog::ResolveImageAssetPath(
    const std::wstring& manifestPath,
    uint32_t imageId,
    std::wstring* outImagePath,
    std::string* outError) {
    if (outImagePath) {
        outImagePath->clear();
    }
    if (outError) {
        outError->clear();
    }
    if (manifestPath.empty()) {
        if (outError) {
            *outError = "manifest path is empty";
        }
        return false;
    }

    const std::wstring key = BuildCacheKey(manifestPath);
    const std::filesystem::path manifestFile(manifestPath);
    const auto writeTime = ReadWriteTime(manifestFile);

    std::lock_guard<std::mutex> guard(CatalogMutex());
    auto& cache = CatalogCache();
    CacheEntry& entry = cache[key];
    if (!entry.loaded || entry.lastWriteTime != writeTime) {
        entry.lastWriteTime = writeTime;
        LoadAssetsFromManifest(manifestPath, &entry);
    }
    if (entry.imagePaths.empty()) {
        if (outError) {
            *outError = entry.error.empty() ? "image assets not found" : entry.error;
        }
        return false;
    }

    const size_t index = static_cast<size_t>(imageId % static_cast<uint32_t>(entry.imagePaths.size()));
    if (outImagePath) {
        *outImagePath = entry.imagePaths[index];
    }
    return true;
}

} // namespace mousefx::wasm
