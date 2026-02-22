#include "pch.h"

#include "Platform/windows/Shell/Win32SettingsLauncher.h"

#include <shellapi.h>

namespace mousefx {

namespace {

std::wstring Utf8ToWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (required <= 0) {
        return {};
    }

    std::wstring wide(static_cast<size_t>(required), L'\0');
    const int written = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        static_cast<int>(value.size()),
        wide.data(),
        required);
    if (written != required) {
        return {};
    }
    return wide;
}

} // namespace

bool Win32SettingsLauncher::OpenUrlUtf8(const std::string& url) {
    const std::wstring wideUrl = Utf8ToWide(url);
    if (wideUrl.empty()) {
        return false;
    }

    const HINSTANCE result = ShellExecuteW(nullptr, L"open", wideUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<INT_PTR>(result) > 32;
}

} // namespace mousefx
