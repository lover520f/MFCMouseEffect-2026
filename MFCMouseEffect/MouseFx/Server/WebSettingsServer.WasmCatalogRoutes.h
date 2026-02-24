#pragma once

#include <string>

namespace mousefx {

class AppController;
struct HttpRequest;
struct HttpResponse;

bool HandleWebSettingsWasmCatalogApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp);

} // namespace mousefx
