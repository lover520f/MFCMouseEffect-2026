#include "pch.h"

#include "Platform/windows/System/Win32TextEncoding.h"

#include <windows.h>

namespace mousefx::platform::windows {
namespace {

std::wstring MultiByteToWide(UINT codePage, DWORD flags, const std::string& text) {
    if (text.empty()) {
        return {};
    }

    const int required = ::MultiByteToWideChar(
        codePage,
        flags,
        text.c_str(),
        -1,
        nullptr,
        0);
    if (required <= 0) {
        return {};
    }

    std::wstring out(static_cast<size_t>(required), L'\0');
    const int converted = ::MultiByteToWideChar(
        codePage,
        flags,
        text.c_str(),
        -1,
        out.data(),
        required);
    if (converted <= 0) {
        return {};
    }
    if (!out.empty() && out.back() == L'\0') {
        out.pop_back();
    }
    return out;
}

std::string WideToMultiByte(UINT codePage, DWORD flags, const wchar_t* wide) {
    if (wide == nullptr || *wide == L'\0') {
        return {};
    }

    const int required = ::WideCharToMultiByte(
        codePage,
        flags,
        wide,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr);
    if (required <= 0) {
        return {};
    }

    std::string out(static_cast<size_t>(required), '\0');
    const int converted = ::WideCharToMultiByte(
        codePage,
        flags,
        wide,
        -1,
        out.data(),
        required,
        nullptr,
        nullptr);
    if (converted <= 0) {
        return {};
    }
    if (!out.empty() && out.back() == '\0') {
        out.pop_back();
    }
    return out;
}

} // namespace

std::wstring Win32TextEncoding::Utf8ToWide(const std::string& utf8) {
    return MultiByteToWide(CP_UTF8, 0, utf8);
}

std::string Win32TextEncoding::WideToUtf8(const wchar_t* wide) {
    return WideToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide);
}

std::string Win32TextEncoding::ActiveCodePageToUtf8(const std::string& text) {
    if (text.empty()) {
        return text;
    }
    const std::wstring wide = MultiByteToWide(CP_ACP, 0, text);
    if (wide.empty()) {
        return {};
    }
    return WideToMultiByte(CP_UTF8, 0, wide.c_str());
}

} // namespace mousefx::platform::windows

