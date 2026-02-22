#include "pch.h"
#include "StringUtils.h"

#include "Platform/PlatformTextEncoding.h"

namespace mousefx {

std::string TrimAscii(std::string s) {
    auto is_space = [](unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    };
    size_t b = 0;
    while (b < s.size() && is_space((unsigned char)s[b])) b++;
    size_t e = s.size();
    while (e > b && is_space((unsigned char)s[e - 1])) e--;
    if (b == 0 && e == s.size()) return s;
    return s.substr(b, e - b);
}

std::string ToLowerAscii(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') out.push_back(static_cast<char>(c - 'A' + 'a'));
        else out.push_back(c);
    }
    return out;
}

std::wstring Utf8ToWString(const std::string& s) {
    return platform::Utf8ToWide(s);
}

std::string Utf16ToUtf8(const wchar_t* ws) {
    return platform::WideToUtf8(ws);
}

bool IsValidUtf8(const std::string& s) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s.data());
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = p[i];
        if (c < 0x80) { i++; continue; }
        if ((c >> 5) == 0x6) {
            if (i + 1 >= s.size()) return false;
            if ((p[i + 1] & 0xC0) != 0x80) return false;
            i += 2; continue;
        }
        if ((c >> 4) == 0xE) {
            if (i + 2 >= s.size()) return false;
            if ((p[i + 1] & 0xC0) != 0x80 || (p[i + 2] & 0xC0) != 0x80) return false;
            i += 3; continue;
        }
        if ((c >> 3) == 0x1E) {
            if (i + 3 >= s.size()) return false;
            if ((p[i + 1] & 0xC0) != 0x80 || (p[i + 2] & 0xC0) != 0x80 || (p[i + 3] & 0xC0) != 0x80) return false;
            i += 4; continue;
        }
        return false;
    }
    return true;
}

std::string EnsureUtf8(const std::string& s) {
    if (s.empty()) return s;
    if (IsValidUtf8(s)) return s;
    return platform::ActiveCodePageToUtf8(s);
}

} // namespace mousefx
