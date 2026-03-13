#pragma once

#include <string>

namespace mousefx {

struct HttpRequest;
struct HttpResponse;
class AppController;

bool HandleWebSettingsTestEffectsProfileApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp);

} // namespace mousefx
