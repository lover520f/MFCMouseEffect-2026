#pragma once

#include "MouseFx/Core/Json/JsonFacade.h"

#include <cstdint>
#include <string>

namespace mousefx {
struct HttpResponse;
}

namespace mousefx::platform::scaffold {

struct SettingsRoute;

struct RuntimeState {
    std::string uiLanguage = "zh-CN";
    std::string theme = "scaffold";
    std::string holdFollowMode = "precise";
};

using json = nlohmann::json;

void SetJsonResponse(HttpResponse& resp, int statusCode, const json& payload);
void SetPlainResponse(HttpResponse& resp, int statusCode, const std::string& body);

json BuildHealthJson(const SettingsRoute& route);
json BuildSchemaJson(const SettingsRoute& route);
json BuildStateJson(
    const SettingsRoute& route,
    const RuntimeState& state,
    bool trayAvailable,
    bool backgroundMode,
    uint64_t revision);
json BuildErrorJson(const std::string& code, const std::string& message);

bool ParseStatePatch(
    const std::string& body,
    const RuntimeState& current,
    RuntimeState* outState,
    json* outError);

} // namespace mousefx::platform::scaffold
