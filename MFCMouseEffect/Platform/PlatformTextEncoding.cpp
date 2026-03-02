#include "pch.h"

#include "Platform/PlatformTextEncoding.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32TextEncoding.h"
#else
#include <climits>
#include <cwchar>
#endif

namespace mousefx::platform {

#if !defined(_WIN32)
namespace {

bool IsUtf8ContinuationByte(unsigned char value) {
    return (value & 0xC0u) == 0x80u;
}

bool AppendCodePointToWide(uint32_t codePoint, std::wstring* out) {
    if (!out) {
        return false;
    }

#if WCHAR_MAX <= 0xFFFF
    if (codePoint <= 0xFFFFu) {
        if (codePoint >= 0xD800u && codePoint <= 0xDFFFu) {
            return false;
        }
        out->push_back(static_cast<wchar_t>(codePoint));
        return true;
    }
    if (codePoint > 0x10FFFFu) {
        return false;
    }
    codePoint -= 0x10000u;
    const wchar_t high = static_cast<wchar_t>(0xD800u + (codePoint >> 10));
    const wchar_t low = static_cast<wchar_t>(0xDC00u + (codePoint & 0x3FFu));
    out->push_back(high);
    out->push_back(low);
    return true;
#else
    if (codePoint > 0x10FFFFu) {
        return false;
    }
    out->push_back(static_cast<wchar_t>(codePoint));
    return true;
#endif
}

bool AppendCodePointToUtf8(uint32_t codePoint, std::string* out) {
    if (!out) {
        return false;
    }
    if (codePoint > 0x10FFFFu) {
        return false;
    }
    if (codePoint >= 0xD800u && codePoint <= 0xDFFFu) {
        return false;
    }

    if (codePoint <= 0x7Fu) {
        out->push_back(static_cast<char>(codePoint));
        return true;
    }
    if (codePoint <= 0x7FFu) {
        out->push_back(static_cast<char>(0xC0u | (codePoint >> 6)));
        out->push_back(static_cast<char>(0x80u | (codePoint & 0x3Fu)));
        return true;
    }
    if (codePoint <= 0xFFFFu) {
        out->push_back(static_cast<char>(0xE0u | (codePoint >> 12)));
        out->push_back(static_cast<char>(0x80u | ((codePoint >> 6) & 0x3Fu)));
        out->push_back(static_cast<char>(0x80u | (codePoint & 0x3Fu)));
        return true;
    }

    out->push_back(static_cast<char>(0xF0u | (codePoint >> 18)));
    out->push_back(static_cast<char>(0x80u | ((codePoint >> 12) & 0x3Fu)));
    out->push_back(static_cast<char>(0x80u | ((codePoint >> 6) & 0x3Fu)));
    out->push_back(static_cast<char>(0x80u | (codePoint & 0x3Fu)));
    return true;
}

bool DecodeUtf8(const std::string& utf8, std::wstring* out) {
    if (!out) {
        return false;
    }

    out->clear();
    out->reserve(utf8.size());
    const size_t size = utf8.size();
    size_t index = 0;
    while (index < size) {
        const unsigned char first = static_cast<unsigned char>(utf8[index]);
        uint32_t codePoint = 0;
        size_t width = 0;
        uint32_t minimum = 0;

        if (first <= 0x7Fu) {
            codePoint = first;
            width = 1;
            minimum = 0;
        } else if ((first & 0xE0u) == 0xC0u) {
            codePoint = first & 0x1Fu;
            width = 2;
            minimum = 0x80u;
        } else if ((first & 0xF0u) == 0xE0u) {
            codePoint = first & 0x0Fu;
            width = 3;
            minimum = 0x800u;
        } else if ((first & 0xF8u) == 0xF0u) {
            codePoint = first & 0x07u;
            width = 4;
            minimum = 0x10000u;
        } else {
            return false;
        }

        if (index + width > size) {
            return false;
        }
        for (size_t offset = 1; offset < width; ++offset) {
            const unsigned char next = static_cast<unsigned char>(utf8[index + offset]);
            if (!IsUtf8ContinuationByte(next)) {
                return false;
            }
            codePoint = (codePoint << 6) | static_cast<uint32_t>(next & 0x3Fu);
        }
        if (codePoint < minimum) {
            return false;
        }
        if (codePoint > 0x10FFFFu) {
            return false;
        }
        if (codePoint >= 0xD800u && codePoint <= 0xDFFFu) {
            return false;
        }
        if (!AppendCodePointToWide(codePoint, out)) {
            return false;
        }
        index += width;
    }
    return true;
}

std::wstring Utf8ToWideFallback(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    std::wstring converted;
    if (!DecodeUtf8(utf8, &converted)) {
        return {};
    }
    return converted;
}

std::string WideToUtf8Fallback(const wchar_t* wide) {
    if (wide == nullptr || *wide == L'\0') {
        return {};
    }

    std::string utf8;
    const size_t length = std::wcslen(wide);
    utf8.reserve(length * 2);

    for (size_t index = 0; index < length; ++index) {
        uint32_t codePoint = 0;
        const uint32_t current = static_cast<uint32_t>(wide[index]);

#if WCHAR_MAX <= 0xFFFF
        if (current >= 0xD800u && current <= 0xDBFFu) {
            if (index + 1 >= length) {
                return {};
            }
            const uint32_t low = static_cast<uint32_t>(wide[index + 1]);
            if (low < 0xDC00u || low > 0xDFFFu) {
                return {};
            }
            codePoint = ((current - 0xD800u) << 10) + (low - 0xDC00u) + 0x10000u;
            ++index;
        } else if (current >= 0xDC00u && current <= 0xDFFFu) {
            return {};
        } else {
            codePoint = current;
        }
#else
        if (current >= 0xD800u && current <= 0xDFFFu) {
            return {};
        }
        codePoint = current;
#endif

        if (!AppendCodePointToUtf8(codePoint, &utf8)) {
            return {};
        }
    }

    return utf8;
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
