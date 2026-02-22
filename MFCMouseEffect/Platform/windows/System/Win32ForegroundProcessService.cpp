#include "pch.h"

#include "Platform/windows/System/Win32ForegroundProcessService.h"

#include <windows.h>

#include <string>

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

std::string Win32ForegroundProcessService::CurrentProcessBaseName() {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint64_t nowTickMs = static_cast<uint64_t>(GetTickCount64());
    if ((nowTickMs - lastCheckTickMs_) < kCacheIntervalMs) {
        return lastProcessBaseName_;
    }
    lastCheckTickMs_ = nowTickMs;
    lastProcessBaseName_ = QueryForegroundProcessBaseName();
    return lastProcessBaseName_;
}

std::string Win32ForegroundProcessService::QueryForegroundProcessBaseName() {
    const HWND hwnd = GetForegroundWindow();
    if (!hwnd || !IsWindow(hwnd)) {
        return {};
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) {
        return {};
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        return {};
    }

    std::wstring fullPath;
    fullPath.resize(1024);
    DWORD size = static_cast<DWORD>(fullPath.size());
    const BOOL ok = QueryFullProcessImageNameW(process, 0, fullPath.data(), &size);
    CloseHandle(process);
    if (!ok || size == 0) {
        return {};
    }

    fullPath.resize(static_cast<size_t>(size));
    const size_t slashPos = fullPath.find_last_of(L"\\/");
    const std::wstring baseName = (slashPos == std::wstring::npos)
        ? fullPath
        : fullPath.substr(slashPos + 1);
    std::string utf8 = Utf16ToUtf8(baseName.c_str());
    utf8 = ToLowerAscii(TrimAscii(utf8));
    return utf8;
}

} // namespace mousefx
