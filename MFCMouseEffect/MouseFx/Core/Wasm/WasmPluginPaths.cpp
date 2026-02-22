#include "pch.h"

#include "WasmPluginPaths.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "Platform/PlatformRuntimeEnvironment.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <set>
#include <system_error>

namespace mousefx::wasm {

namespace {

std::wstring JoinPath(const std::wstring& base, const wchar_t* child) {
    if (base.empty() || !child || child[0] == L'\0') {
        return base;
    }
    std::filesystem::path p(base);
    p /= child;
    return p.wstring();
}

std::filesystem::path ModuleDirectory() {
    const std::wstring exeDir = platform::GetExecutableDirectoryW();
    if (exeDir.empty()) {
        return {};
    }
    return std::filesystem::path(exeDir);
}

std::filesystem::path NormalizePath(const std::filesystem::path& path) {
    std::error_code ec;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
    if (!ec) {
        return canonical.lexically_normal();
    }
    return path.lexically_normal();
}

std::wstring BuildPathKey(const std::filesystem::path& path) {
    std::wstring key = NormalizePath(path).wstring();
    std::transform(key.begin(), key.end(), key.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return key;
}

bool ExistsDirectory(const std::filesystem::path& path) {
    std::error_code ec;
    return !path.empty() && std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec) && !ec;
}

void AddRootIfValid(
    const std::filesystem::path& root,
    std::set<std::wstring>* seenKeys,
    std::vector<std::wstring>* roots) {
    if (!seenKeys || !roots || !ExistsDirectory(root)) {
        return;
    }
    const std::wstring key = BuildPathKey(root);
    if (!seenKeys->insert(key).second) {
        return;
    }
    roots->push_back(NormalizePath(root).wstring());
}

void AddRootUnique(
    const std::filesystem::path& root,
    std::set<std::wstring>* seenKeys,
    std::vector<std::wstring>* roots) {
    if (!seenKeys || !roots || root.empty()) {
        return;
    }
    const std::wstring key = BuildPathKey(root);
    if (!seenKeys->insert(key).second) {
        return;
    }
    roots->push_back(NormalizePath(root).wstring());
}

std::filesystem::path ResolveExecutablePluginRootPath() {
    const std::filesystem::path exeDir = ModuleDirectory();
    if (exeDir.empty()) {
        return {};
    }
    return exeDir / L"plugins" / L"wasm";
}

#ifdef _DEBUG
std::filesystem::path ResolveDevelopmentTemplateDistPath() {
    std::filesystem::path cursor = ModuleDirectory();
    if (cursor.empty()) {
        return {};
    }

    constexpr int kMaxDepth = 8;
    for (int depth = 0; depth < kMaxDepth; ++depth) {
        const std::filesystem::path candidate = cursor / L"examples" / L"wasm-plugin-template" / L"dist";
        const std::filesystem::path marker = candidate / L"plugin.json";
        std::error_code ec;
        if (std::filesystem::exists(marker, ec) && !ec) {
            return candidate;
        }

        if (cursor == cursor.root_path()) {
            break;
        }
        const std::filesystem::path parent = cursor.parent_path();
        if (parent.empty() || parent == cursor) {
            break;
        }
        cursor = parent;
    }
    return {};
}
#endif

} // namespace

std::wstring WasmPluginPaths::ResolvePrimaryPluginRoot() {
    const std::wstring configDir = ResolveConfigDirectory();
    std::wstring root = JoinPath(configDir, L"plugins");
    root = JoinPath(root, L"wasm");

    if (root.empty()) {
        return root;
    }
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(root), ec);
    return root;
}

std::vector<std::wstring> WasmPluginPaths::ResolveSearchRoots(const std::wstring& configuredRoot) {
    std::vector<std::wstring> roots{};
    std::set<std::wstring> seenKeys{};

    const std::wstring primary = ResolvePrimaryPluginRoot();
    if (!primary.empty()) {
        AddRootIfValid(std::filesystem::path(primary), &seenKeys, &roots);
    }

    AddRootIfValid(ResolveExecutablePluginRootPath(), &seenKeys, &roots);

#ifdef _DEBUG
    AddRootIfValid(ResolveDevelopmentTemplateDistPath(), &seenKeys, &roots);
#endif

    if (!configuredRoot.empty()) {
        AddRootUnique(std::filesystem::path(configuredRoot), &seenKeys, &roots);
    }

    return roots;
}

std::wstring WasmPluginPaths::ResolveManifestRelativePath(
    const std::wstring& manifestPath,
    const std::wstring& relativePath) {
    if (manifestPath.empty() || relativePath.empty()) {
        return {};
    }
    const std::filesystem::path relPath(relativePath);
    if (relPath.is_absolute()) {
        return {};
    }
    const std::filesystem::path manifestFile(manifestPath);
    const std::filesystem::path baseDir = manifestFile.parent_path();
    std::filesystem::path resolved = baseDir / relPath;
    resolved = resolved.lexically_normal();
    return resolved.wstring();
}

std::wstring WasmPluginPaths::ResolveEntryWasmPath(const std::wstring& manifestPath, const PluginManifest& manifest) {
    return ResolveManifestRelativePath(manifestPath, manifest.entryWasm);
}

} // namespace mousefx::wasm
