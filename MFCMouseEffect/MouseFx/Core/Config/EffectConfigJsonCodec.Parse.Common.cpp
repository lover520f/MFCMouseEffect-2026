#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::config_json::parse_internal {

bool TryUtf8ToWide(const std::string& utf8, std::wstring* out) {
    if (out == nullptr) {
        return false;
    }
    if (utf8.empty()) {
        out->clear();
        return true;
    }

    const std::wstring wide = Utf8ToWString(utf8);
    if (wide.empty() && !utf8.empty()) {
        return false;
    }
    *out = wide;
    return true;
}

} // namespace mousefx::config_json::parse_internal
