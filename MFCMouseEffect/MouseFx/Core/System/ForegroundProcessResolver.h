#pragma once

#include <windows.h>

#include <chrono>
#include <string>

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

// Resolves foreground process base-name with a short cache window to avoid
// querying process info for every mouse event.
class ForegroundProcessResolver final {
public:
    std::string CurrentProcessBaseName() {
        return CurrentProcessBaseName(NowMs());
    }

    std::string CurrentProcessBaseName(uint64_t nowTickMs) {
        if ((nowTickMs - lastCheckTickMs_) < kCacheIntervalMs) {
            return lastProcessBaseName_;
        }
        lastCheckTickMs_ = nowTickMs;
        lastProcessBaseName_ = QueryForegroundProcessBaseName();
        return lastProcessBaseName_;
    }

private:
    static uint64_t NowMs() {
        using namespace std::chrono;
        const auto now = steady_clock::now().time_since_epoch();
        return static_cast<uint64_t>(duration_cast<milliseconds>(now).count());
    }

    static std::string QueryForegroundProcessBaseName() {
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

    static constexpr uint64_t kCacheIntervalMs = 200;
    uint64_t lastCheckTickMs_ = 0;
    std::string lastProcessBaseName_{};
};

} // namespace mousefx
