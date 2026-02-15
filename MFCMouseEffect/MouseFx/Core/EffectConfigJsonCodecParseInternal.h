#pragma once

#include "EffectConfigJsonCodec.h"

#include <string>

namespace mousefx::config_json::parse_internal {

template<typename T>
inline T GetOr(const nlohmann::json& j, const char* key, T defaultValue) {
    if (j.contains(key) && !j[key].is_null()) {
        try {
            return j[key].get<T>();
        } catch (...) {
        }
    }
    return defaultValue;
}

inline Argb GetColorOr(const nlohmann::json& j, const char* key, Argb defaultValue) {
    if (j.contains(key) && j[key].is_string()) {
        return ArgbFromHex(j[key].get<std::string>());
    }
    return defaultValue;
}

bool TryUtf8ToWide(const std::string& utf8, std::wstring* out);

void ParseInputIndicator(const nlohmann::json& root, EffectConfig& config);
void ParseTrailParams(const nlohmann::json& root, EffectConfig& config);
void ParseTrailProfiles(const nlohmann::json& root, EffectConfig& config);
void ParseEffects(const nlohmann::json& root, EffectConfig& config);

} // namespace mousefx::config_json::parse_internal
