#pragma once

#include <string>

namespace mousefx {

class AppController;
struct HttpRequest;
struct HttpResponse;

bool HandleWebSettingsWasmRuntimeActionApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp);

} // namespace mousefx
