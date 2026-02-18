#include "pch.h"

#include "WasmPluginPaths.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"

#include <filesystem>
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

std::vector<std::wstring> WasmPluginPaths::ResolveSearchRoots() {
    std::vector<std::wstring> roots;
    const std::wstring primary = ResolvePrimaryPluginRoot();
    if (!primary.empty()) {
        roots.push_back(primary);
    }
    return roots;
}

std::wstring WasmPluginPaths::ResolveEntryWasmPath(const std::wstring& manifestPath, const PluginManifest& manifest) {
    if (manifestPath.empty() || manifest.entryWasm.empty()) {
        return {};
    }

    const std::filesystem::path manifestFile(manifestPath);
    const std::filesystem::path baseDir = manifestFile.parent_path();
    std::filesystem::path entryPath = baseDir / std::filesystem::path(manifest.entryWasm);
    entryPath = entryPath.lexically_normal();
    return entryPath.wstring();
}

} // namespace mousefx::wasm

