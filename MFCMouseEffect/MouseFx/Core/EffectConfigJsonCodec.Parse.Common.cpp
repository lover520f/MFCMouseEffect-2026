#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

namespace mousefx::config_json::parse_internal {

bool TryUtf8ToWide(const std::string& utf8, std::wstring* out) {
    if (out == nullptr) {
        return false;
    }
    if (utf8.empty()) {
        out->clear();
        return true;
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (len <= 0) {
        return false;
    }

    std::wstring wide(static_cast<size_t>(len), L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), len) <= 0) {
        return false;
    }

    if (!wide.empty() && wide.back() == L'\0') {
        wide.pop_back();
    }
    *out = wide;
    return true;
}

} // namespace mousefx::config_json::parse_internal
