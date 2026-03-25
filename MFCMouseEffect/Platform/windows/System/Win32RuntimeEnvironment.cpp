// Win32RuntimeEnvironment.cpp -- Win32 runtime probe and path helpers

#include "pch.h"
#include "Platform/windows/System/Win32RuntimeEnvironment.h"

#include <windows.h>
#include <shlobj.h>
#include <filesystem>

namespace mousefx::platform::windows {

std::wstring GetExecutableDirectoryW() {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring p(exePath);
    const size_t pos = p.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return L".";
    return p.substr(0, pos);
}

std::wstring GetParentDirectoryW(const std::wstring& path) {
    const size_t pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return {};
    return path.substr(0, pos);
}

std::wstring GetPreferredConfigDirectoryW() {
    PWSTR appDataPath = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath))) {
        return {};
    }

    std::wstring dir = std::wstring(appDataPath) + L"\\MFCMouseEffect";
    CoTaskMemFree(appDataPath);
    return dir;
}

RuntimeProbeResult ProbeDawnRuntimeOnce() {
#if !defined(MFX_ENABLE_WINDOWS_GPU_EFFECTS) || !MFX_ENABLE_WINDOWS_GPU_EFFECTS
    RuntimeProbeResult r{};
    r.available = false;
    r.reason = "dawn_runtime_disabled_by_build";
    return r;
#else
    const std::wstring exeDir = GetExecutableDirectoryW();
    const std::filesystem::path primary = std::filesystem::path(exeDir) / L"webgpu_dawn.dll";
    const std::filesystem::path fallback = std::filesystem::path(exeDir) / L"Runtime" / L"Dawn" / L"webgpu_dawn.dll";
    const std::wstring repoRoot = GetParentDirectoryW(GetParentDirectoryW(exeDir));
    const std::filesystem::path repoRuntime = std::filesystem::path(repoRoot) / L"MFCMouseEffect" / L"Runtime" / L"Dawn" / L"webgpu_dawn.dll";

    auto tryLoad = [](const std::filesystem::path& p) -> bool {
        if (p.empty()) return false;
        if (!std::filesystem::exists(p)) return false;
        HMODULE h = LoadLibraryW(p.c_str());
        if (!h) return false;
        FreeLibrary(h);
        return true;
    };

    RuntimeProbeResult r{};
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
#endif
}

} // namespace mousefx::platform::windows
