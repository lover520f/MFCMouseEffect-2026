#pragma once

#include <string>

namespace mousefx {

struct HttpRequest;
struct HttpResponse;
class AppController;

bool HandleWebSettingsTestEffectsApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp);

} // namespace mousefx
