#pragma once

#include <string>

namespace mousefx {

struct HttpRequest;
struct HttpResponse;
class AppController;

bool HandleWebSettingsTestEffectsOverlayApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp);

} // namespace mousefx
