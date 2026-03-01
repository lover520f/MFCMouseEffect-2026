#include "pch.h"
#include "WebSettingsServer.TestRouteCommon.h"

#include <cstdlib>
#include <cmath>
#include <limits>

#include "MouseFx/Server/HttpServer.h"

namespace mousefx {
namespace websettings_test_routes {
namespace {

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLowerAscii(lhs[i]) != ToLowerAscii(rhs[i])) {
            return false;
        }
    }
    return true;
}

} // namespace

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = body;
}

void SetPlainResponse(HttpResponse& resp, int code, const std::string& body) {
    resp.statusCode = code;
    resp.contentType = "text/plain; charset=utf-8";
    resp.body = body;
}

nlohmann::json ParseObjectOrEmpty(const std::string& body) {
    if (body.empty()) {
        return nlohmann::json::object();
    }
    try {
        nlohmann::json parsed = nlohmann::json::parse(body);
        if (parsed.is_object()) {
            return parsed;
        }
    } catch (...) {
    }
    return nlohmann::json::object();
}

bool IsEnabledByEnv(const char* name) {
    const char* raw = std::getenv(name);
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }

    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

bool ParseBooleanOrDefault(const nlohmann::json& payload, const char* key, bool defaultValue) {
    if (!payload.contains(key)) {
        return defaultValue;
    }
    if (payload[key].is_boolean()) {
        return payload[key].get<bool>();
    }
    if (payload[key].is_number_integer()) {
        return payload[key].get<int>() != 0;
    }
    return defaultValue;
}

int32_t ParseInt32OrDefault(const nlohmann::json& payload, const char* key, int32_t defaultValue) {
    if (!payload.contains(key) || !payload[key].is_number_integer()) {
        return defaultValue;
    }
    const int64_t raw = payload[key].get<int64_t>();
    if (raw < static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
        return std::numeric_limits<int32_t>::min();
    }
    if (raw > static_cast<int64_t>(std::numeric_limits<int32_t>::max())) {
        return std::numeric_limits<int32_t>::max();
    }
    return static_cast<int32_t>(raw);
}

uint8_t ParseButtonOrDefault(const nlohmann::json& payload, const char* key, uint8_t defaultValue) {
    if (!payload.contains(key) || !payload[key].is_number_integer()) {
        return defaultValue;
    }
    const int64_t raw = payload[key].get<int64_t>();
    if (raw <= 0) {
        return 0;
    }
    if (raw > static_cast<int64_t>(std::numeric_limits<uint8_t>::max())) {
        return std::numeric_limits<uint8_t>::max();
    }
    return static_cast<uint8_t>(raw);
}

float ParseFloatOrDefault(const nlohmann::json& payload, const char* key, float defaultValue) {
    if (!payload.contains(key) || !payload[key].is_number()) {
        return defaultValue;
    }
    const double raw = payload[key].get<double>();
    if (!std::isfinite(raw)) {
        return defaultValue;
    }
    return static_cast<float>(raw);
}

} // namespace websettings_test_routes
} // namespace mousefx
