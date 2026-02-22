#pragma once

#include <string>

namespace mousefx::platform::windows {

class Win32TextEncoding final {
public:
    static std::wstring Utf8ToWide(const std::string& utf8);
    static std::string WideToUtf8(const wchar_t* wide);
    static std::string ActiveCodePageToUtf8(const std::string& text);
};

} // namespace mousefx::platform::windows

