#include "pch.h"

#include "Platform/PlatformRuntimeEnvironment.h"

#include "Platform/PlatformTextEncoding.h"
#include "Platform/PlatformTarget.h"

#if MFX_PLATFORM_WINDOWS
#include "Platform/windows/System/Win32RuntimeEnvironment.h"
#endif

#if MFX_PLATFORM_MACOS
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

#if MFX_PLATFORM_LINUX
#include <unistd.h>
#endif

#include <array>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace {

std::wstring BuildConfigDirFromHome(const char* home, const char* subPath) {
    if (home == nullptr || home[0] == '\0' || subPath == nullptr || subPath[0] == '\0') {
        return {};
    }

    std::filesystem::path dir(home);
    dir /= subPath;
    return mousefx::platform::Utf8ToWide(dir.generic_string());
}

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX
std::wstring ReadExecutableDirFromProcPath(const char* procPath) {
    if (procPath == nullptr || procPath[0] == '\0') {
        return {};
    }

    std::array<char, 4096> buf{};
    const ssize_t len = readlink(procPath, buf.data(), buf.size() - 1);
    if (len <= 0) {
        return {};
    }

    buf[static_cast<size_t>(len)] = '\0';
    std::filesystem::path exePath(buf.data());
    return mousefx::platform::Utf8ToWide(exePath.parent_path().generic_string());
}
#endif

} // namespace

namespace mousefx::platform {

std::wstring GetExecutableDirectoryW() {
#if MFX_PLATFORM_WINDOWS
    return windows::GetExecutableDirectoryW();
#elif MFX_PLATFORM_MACOS
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    if (size > 0) {
        std::string path(size, '\0');
        if (_NSGetExecutablePath(path.data(), &size) == 0) {
            std::filesystem::path exePath(path.c_str());
            return mousefx::platform::Utf8ToWide(exePath.parent_path().generic_string());
        }
    }
    return ReadExecutableDirFromProcPath("/proc/self/exe");
#elif MFX_PLATFORM_LINUX
    return ReadExecutableDirFromProcPath("/proc/self/exe");
#else
    return {};
#endif
}

std::wstring GetParentDirectoryW(const std::wstring& path) {
#if MFX_PLATFORM_WINDOWS
    return windows::GetParentDirectoryW(path);
#else
    if (path.empty()) {
        return {};
    }
    return std::filesystem::path(path).parent_path().wstring();
#endif
}

std::wstring GetPreferredConfigDirectoryW() {
#if MFX_PLATFORM_WINDOWS
    return windows::GetPreferredConfigDirectoryW();
#elif MFX_PLATFORM_MACOS
    return BuildConfigDirFromHome(std::getenv("HOME"), "Library/Application Support/MFCMouseEffect");
#elif MFX_PLATFORM_LINUX
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg != nullptr && xdg[0] != '\0') {
        std::filesystem::path dir(xdg);
        dir /= "MFCMouseEffect";
        return Utf8ToWide(dir.generic_string());
    }
    return BuildConfigDirFromHome(std::getenv("HOME"), ".config/MFCMouseEffect");
#else
    return {};
#endif
}

RuntimeProbeResult ProbeDawnRuntimeOnce() {
#if MFX_PLATFORM_WINDOWS
    return windows::ProbeDawnRuntimeOnce();
#else
    RuntimeProbeResult result{};
    result.available = false;
    result.reason = "runtime_probe_not_supported";
    return result;
#endif
}

} // namespace mousefx::platform
