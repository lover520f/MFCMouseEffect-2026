#pragma once

#include <string>

namespace mousefx {

// Trim leading/trailing ASCII whitespace (space, tab, CR, LF).
std::string TrimAscii(std::string s);

// Convert ASCII letters to lowercase (leaves non-ASCII untouched).
std::string ToLowerAscii(const std::string& s);

// Convert UTF-8 string to wide string (Windows API).
std::wstring Utf8ToWString(const std::string& s);

// Convert wide string to UTF-8 string (Windows API).
std::string Utf16ToUtf8(const wchar_t* ws);

// Check if a string is valid UTF-8.
bool IsValidUtf8(const std::string& s);

// Ensure a string is valid UTF-8 (re-encode from ACP if needed).
std::string EnsureUtf8(const std::string& s);

} // namespace mousefx
