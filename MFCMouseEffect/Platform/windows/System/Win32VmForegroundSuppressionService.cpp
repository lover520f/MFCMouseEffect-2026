#include "pch.h"

#include "Platform/windows/System/Win32VmForegroundSuppressionService.h"

#include <windows.h>

#include <algorithm>
#include <cwctype>
#include <string>

namespace mousefx {

bool Win32VmForegroundSuppressionService::ShouldSuppress(uint64_t nowTickMs) {
    if ((nowTickMs - lastCheckTickMs_) < kCheckIntervalMs) {
        return lastResult_;
    }
    lastCheckTickMs_ = nowTickMs;
    lastResult_ = IsVmForegroundWindow();
    return lastResult_;
}

bool Win32VmForegroundSuppressionService::IsVmForegroundWindow() {
    const HWND hwnd = GetForegroundWindow();
    if (!hwnd || !IsWindow(hwnd)) {
        return false;
    }

    std::wstring processName;
    if (TryGetProcessBaseName(hwnd, &processName) && ContainsVmToken(processName)) {
        return true;
    }

    std::wstring className;
    className.resize(256);
    const int classLen = GetClassNameW(hwnd, className.data(), static_cast<int>(className.size()));
    if (classLen > 0) {
        className.resize(static_cast<size_t>(classLen));
        if (ContainsVmToken(className)) {
            return true;
        }
    }

    std::wstring title;
    title.resize(512);
    const int titleLen = GetWindowTextW(hwnd, title.data(), static_cast<int>(title.size()));
    if (titleLen > 0) {
        title.resize(static_cast<size_t>(titleLen));
        if (ContainsVmToken(title)) {
            return true;
        }
    }

    return false;
}

bool Win32VmForegroundSuppressionService::TryGetProcessBaseName(void* rawHwnd, std::wstring* outBaseName) {
    if (!outBaseName) {
        return false;
    }
    outBaseName->clear();

    const HWND hwnd = reinterpret_cast<HWND>(rawHwnd);
    if (!hwnd) {
        return false;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) {
        return false;
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        return false;
    }

    std::wstring fullPath;
    fullPath.resize(1024);
    DWORD size = static_cast<DWORD>(fullPath.size());
    const BOOL ok = QueryFullProcessImageNameW(process, 0, fullPath.data(), &size);
    CloseHandle(process);
    if (!ok || size == 0) {
        return false;
    }

    fullPath.resize(static_cast<size_t>(size));
    const size_t slashPos = fullPath.find_last_of(L"\\/");
    *outBaseName = (slashPos == std::wstring::npos) ? fullPath : fullPath.substr(slashPos + 1);
    return !outBaseName->empty();
}

bool Win32VmForegroundSuppressionService::ContainsVmToken(const std::wstring& input) {
    if (input.empty()) {
        return false;
    }
    const std::wstring lower = ToLower(input);
    for (const wchar_t* token : kVmTokens) {
        if (lower.find(token) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

std::wstring Win32VmForegroundSuppressionService::ToLower(const std::wstring& input) {
    std::wstring out = input;
    std::transform(out.begin(), out.end(), out.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return out;
}

} // namespace mousefx
