#include "pch.h"

#include "Platform/PlatformTextEncoding.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32TextEncoding.h"
#else
#include <codecvt>
#include <locale>
#endif

namespace mousefx::platform {

#if !defined(_WIN32)
namespace {

std::wstring Utf8ToWideFallback(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(utf8);
    } catch (...) {
        return {};
    }
}

std::string WideToUtf8Fallback(const wchar_t* wide) {
    if (wide == nullptr || *wide == L'\0') {
        return {};
    }
    try {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wide);
    } catch (...) {
        return {};
    }
}

} // namespace
#endif

std::wstring Utf8ToWide(const std::string& utf8) {
#if defined(_WIN32)
    return windows::Win32TextEncoding::Utf8ToWide(utf8);
#else
    return Utf8ToWideFallback(utf8);
#endif
}

std::string WideToUtf8(const wchar_t* wide) {
#if defined(_WIN32)
    return windows::Win32TextEncoding::WideToUtf8(wide);
#else
    return WideToUtf8Fallback(wide);
#endif
}

std::string ActiveCodePageToUtf8(const std::string& text) {
#if defined(_WIN32)
    return windows::Win32TextEncoding::ActiveCodePageToUtf8(text);
#else
    return text;
#endif
}

} // namespace mousefx::platform

