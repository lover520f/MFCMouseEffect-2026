#include "pch.h"

#include "ConfigPathResolver.h"

#include <shlobj.h>

namespace mousefx {

std::wstring ResolveConfigDirectory() {
    wchar_t exePath[MAX_PATH] = {};

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
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring p(exePath);
    const size_t pos = p.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        return p.substr(0, pos);
    }
    return L".";
}

} // namespace mousefx

