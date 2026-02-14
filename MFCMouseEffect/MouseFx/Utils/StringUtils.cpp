#include "pch.h"
#include "StringUtils.h"

#include <windows.h>

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
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring out((size_t)len, L'\0');
    int written = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.empty() ? nullptr : &out[0], len);
    if (written <= 0) return {};
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

std::string Utf16ToUtf8(const wchar_t* ws) {
    if (!ws || !*ws) return {};
    int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ws, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string out((size_t)len, '\0');
    int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, ws, -1, out.empty() ? nullptr : &out[0], len, nullptr, nullptr);
    if (written <= 0) return {};
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
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

    int wlen = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return {};
    std::wstring w((size_t)wlen, L'\0');
    int wwritten = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, w.empty() ? nullptr : &w[0], wlen);
    if (wwritten <= 0) return {};
    if (!w.empty() && w.back() == L'\0') w.pop_back();

    int ulen = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ulen <= 0) return {};
    std::string out((size_t)ulen, '\0');
    int uwritten = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.empty() ? nullptr : &out[0], ulen, nullptr, nullptr);
    if (uwritten <= 0) return {};
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

} // namespace mousefx
