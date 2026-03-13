#pragma once

#include <string>

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

class AppController;
struct HttpResponse;

namespace websettings_wasm_routes {

void SetJsonResponse(HttpResponse& resp, const std::string& body);
nlohmann::json ParseObjectOrEmpty(const std::string& body);
std::string ParseManifestPathUtf8(const nlohmann::json& payload);
std::string ParseInitialPathUtf8(const nlohmann::json& payload);
bool IsSameManifestPath(const std::wstring& expected, const std::wstring& actual);
nlohmann::json BuildWasmResponse(AppController* controller, bool ok);
nlohmann::json BuildWasmActionResponse(
    AppController* controller,
    bool ok,
    const std::string& defaultError,
    const std::string& defaultErrorCode = {});

} // namespace websettings_wasm_routes
} // namespace mousefx
