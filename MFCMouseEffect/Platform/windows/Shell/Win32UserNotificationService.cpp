#include "pch.h"

#include "Platform/windows/Shell/Win32UserNotificationService.h"

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

void Win32UserNotificationService::ShowWarning(const std::string& titleUtf8, const std::string& messageUtf8) {
    const std::wstring title = Utf8ToWide(titleUtf8.empty() ? "MFCMouseEffect" : titleUtf8);
    const std::wstring message = Utf8ToWide(messageUtf8);
    MessageBoxW(
        nullptr,
        message.empty() ? L"(empty)" : message.c_str(),
        title.empty() ? L"MFCMouseEffect" : title.c_str(),
        MB_OK | MB_ICONWARNING);
}

} // namespace mousefx
