#pragma once

#include <windows.h>

#include <array>
#include <cstdint>
#include <cwctype>
#include <string>

namespace mousefx {

class VmForegroundDetector final {
public:
    bool ShouldSuppress(uint64_t nowTickMs) {
        if ((nowTickMs - lastCheckTickMs_) < kCheckIntervalMs) {
            return lastResult_;
        }
        lastCheckTickMs_ = nowTickMs;
        lastResult_ = IsVmForegroundWindow();
        return lastResult_;
    }

private:
    static bool IsVmForegroundWindow() {
        const HWND hwnd = GetForegroundWindow();
        if (!hwnd || !IsWindow(hwnd)) return false;

        std::wstring processName;
        if (TryGetProcessBaseName(hwnd, &processName) && ContainsVmToken(processName)) {
            return true;
        }

        std::wstring className;
        className.resize(256);
        const int classLen = GetClassNameW(hwnd, className.data(), static_cast<int>(className.size()));
        if (classLen > 0) {
            className.resize(static_cast<size_t>(classLen));
            if (ContainsVmToken(className)) return true;
        } else {
            className.clear();
        }

        std::wstring title;
        title.resize(512);
        const int titleLen = GetWindowTextW(hwnd, title.data(), static_cast<int>(title.size()));
        if (titleLen > 0) {
            title.resize(static_cast<size_t>(titleLen));
            if (ContainsVmToken(title)) return true;
        }

        return false;
    }

    static bool TryGetProcessBaseName(HWND hwnd, std::wstring* outBaseName) {
        if (!outBaseName) return false;
        outBaseName->clear();

        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == 0) return false;

        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!process) return false;

        std::wstring fullPath;
        fullPath.resize(1024);
        DWORD size = static_cast<DWORD>(fullPath.size());
        const BOOL ok = QueryFullProcessImageNameW(process, 0, fullPath.data(), &size);
        CloseHandle(process);
        if (!ok || size == 0) return false;

        fullPath.resize(static_cast<size_t>(size));
        const size_t slashPos = fullPath.find_last_of(L"\\/");
        *outBaseName = (slashPos == std::wstring::npos) ? fullPath : fullPath.substr(slashPos + 1);
        return !outBaseName->empty();
    }

    static bool ContainsVmToken(const std::wstring& input) {
        if (input.empty()) return false;
        std::wstring s = ToLower(input);
        for (const wchar_t* token : kVmTokens) {
            if (s.find(token) != std::wstring::npos) {
                return true;
            }
        }
        return false;
    }

    static std::wstring ToLower(const std::wstring& input) {
        std::wstring s = input;
        for (wchar_t& ch : s) {
            ch = static_cast<wchar_t>(towlower(ch));
        }
        return s;
    }

    static constexpr std::array<const wchar_t*, 11> kVmTokens = {
        L"vmware",
        L"virtualbox",
        L"virtual box",
        L"virtualboxvm",
        L"vmconnect",
        L"qemu",
        L"virt-viewer",
        L"remote-viewer",
        L"parallels",
        L"prl_vm",
        L"hyper-v"
    };

    static constexpr uint64_t kCheckIntervalMs = 800;
    uint64_t lastCheckTickMs_ = 0;
    bool lastResult_ = false;
};

} // namespace mousefx

