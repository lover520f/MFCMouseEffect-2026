#include "pch.h"

#include "ConfigPathResolver.h"

#include "Platform/PlatformRuntimeEnvironment.h"

#include <shlobj.h>

namespace mousefx {

std::wstring ResolveConfigDirectory() {
    // Try to use %AppData%\\MFCMouseEffect for Release builds.
#ifndef _DEBUG
    PWSTR appDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath))) {
        std::wstring dir = std::wstring(appDataPath) + L"\\MFCMouseEffect";
        CoTaskMemFree(appDataPath);

        const int res = SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
        if (res == ERROR_SUCCESS || res == ERROR_ALREADY_EXISTS || res == ERROR_FILE_EXISTS) {
            return dir;
        }
    }
#endif

    // Fallback to EXE directory.
    const std::wstring exeDir = platform::GetExecutableDirectoryW();
    if (!exeDir.empty()) {
        return exeDir;
    }
    return L".";
}

std::wstring ResolveLocalDiagDirectory() {
    const std::wstring exeDir = platform::GetExecutableDirectoryW();
    if (exeDir.empty()) {
        return L".\\.local\\diag";
    }
    return exeDir + L"\\.local\\diag";
}

} // namespace mousefx
