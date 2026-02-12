#include "pch.h"

#include "GpuFinalPresentOptIn.h"

#include <fstream>
#include <string>

namespace mousefx::gpu {
namespace {

std::wstring ResolveExeDir() {
    wchar_t modulePath[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return L".";
    std::wstring exePath(modulePath, modulePath + len);
    const size_t pos = exePath.find_last_of(L"\\/");
    return (pos == std::wstring::npos) ? L"." : exePath.substr(0, pos);
}

bool FileExists(const std::wstring& path) {
    std::ifstream fin(path, std::ios::binary);
    return fin.good();
}

} // namespace

bool IsGpuFinalPresentOptInEnabled() {
    const std::wstring exeDir = ResolveExeDir();
    const std::wstring offPath = exeDir + L"\\.local\\diag\\gpu_final_present.off";
    if (FileExists(offPath)) {
        return false;
    }

    // Keep legacy positive marker compatible if users/scripts still create it.
    const std::wstring legacyOptInPath = exeDir + L"\\.local\\diag\\gpu_final_present.optin";
    if (FileExists(legacyOptInPath)) {
        return true;
    }

    // Default on to keep GPU final-present rollout fully self-driven.
    return true;
}

} // namespace mousefx::gpu
