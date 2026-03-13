#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

struct HttpResponse;

namespace websettings_test_routes {

void SetJsonResponse(HttpResponse& resp, const std::string& body);
void SetPlainResponse(HttpResponse& resp, int code, const std::string& body);
nlohmann::json ParseObjectOrEmpty(const std::string& body);
bool IsEnabledByEnv(const char* name);

bool ParseBooleanOrDefault(const nlohmann::json& payload, const char* key, bool defaultValue);
int32_t ParseInt32OrDefault(const nlohmann::json& payload, const char* key, int32_t defaultValue);
uint8_t ParseButtonOrDefault(const nlohmann::json& payload, const char* key, uint8_t defaultValue);
uint32_t ParseUInt32OrDefault(const nlohmann::json& payload, const char* key, uint32_t defaultValue);
float ParseFloatOrDefault(const nlohmann::json& payload, const char* key, float defaultValue);

} // namespace websettings_test_routes
} // namespace mousefx
