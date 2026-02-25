#pragma once

#include <string>

namespace mousefx {

struct HttpRequest;
struct HttpResponse;

bool HandleWebSettingsTestEffectsApiRoute(
    const HttpRequest& req,
    const std::string& path,
    HttpResponse& resp);

} // namespace mousefx
