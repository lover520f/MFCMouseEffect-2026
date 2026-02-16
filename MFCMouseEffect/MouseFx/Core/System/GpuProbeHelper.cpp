// GpuProbeHelper.cpp -- Dawn runtime probe extracted from AppController

#include "pch.h"
#include "GpuProbeHelper.h"

#include <windows.h>
#include <filesystem>

namespace mousefx {

std::wstring GetExeDirW() {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring p(exePath);
    const size_t pos = p.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return L".";
    return p.substr(0, pos);
}

std::wstring ParentDirW(const std::wstring& path) {
    const size_t pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return {};
    return path.substr(0, pos);
}

DawnRuntimeProbeResult ProbeDawnRuntimeOnce() {
    const std::wstring exeDir = GetExeDirW();
    const std::filesystem::path primary = std::filesystem::path(exeDir) / L"webgpu_dawn.dll";
    const std::filesystem::path fallback = std::filesystem::path(exeDir) / L"Runtime" / L"Dawn" / L"webgpu_dawn.dll";
    const std::wstring repoRoot = ParentDirW(ParentDirW(exeDir));
    const std::filesystem::path repoRuntime = std::filesystem::path(repoRoot) / L"MFCMouseEffect" / L"Runtime" / L"Dawn" / L"webgpu_dawn.dll";

    auto tryLoad = [](const std::filesystem::path& p) -> bool {
        if (p.empty()) return false;
        if (!std::filesystem::exists(p)) return false;
        HMODULE h = LoadLibraryW(p.c_str());
        if (!h) return false;
        FreeLibrary(h);
        return true;
    };

    DawnRuntimeProbeResult r{};
    if (tryLoad(primary)) {
        r.available = true;
        r.reason = "dawn_runtime_loaded_from_exe_dir";
        return r;
    }
    if (tryLoad(fallback)) {
        r.available = true;
        r.reason = "dawn_runtime_loaded_from_runtime_fallback_dir";
        return r;
    }
    if (tryLoad(repoRuntime)) {
        r.available = true;
        r.reason = "dawn_runtime_loaded_from_repo_runtime_dir";
        return r;
    }
    r.available = false;
    r.reason = "dawn_runtime_binary_missing_or_load_failed";
    return r;
}

} // namespace mousefx
