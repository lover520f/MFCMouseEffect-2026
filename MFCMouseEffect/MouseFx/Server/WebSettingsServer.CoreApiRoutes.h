#pragma once

#include <functional>
#include <string>

namespace mousefx {

class AppController;
struct HttpRequest;
struct HttpResponse;

bool HandleWebSettingsCoreApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    const std::function<void()>& stopAsync,
    HttpResponse& resp);

} // namespace mousefx
