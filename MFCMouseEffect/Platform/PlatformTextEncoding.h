#pragma once

#include <string>

namespace mousefx::platform {

// Convert UTF-8 text to wide text.
std::wstring Utf8ToWide(const std::string& utf8);

// Convert wide text to UTF-8 text.
std::string WideToUtf8(const wchar_t* wide);

// Convert system active code page text to UTF-8.
std::string ActiveCodePageToUtf8(const std::string& text);

} // namespace mousefx::platform

