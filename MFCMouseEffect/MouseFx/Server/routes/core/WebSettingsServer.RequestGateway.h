#pragma once

#include <functional>
#include <string>

namespace mousefx {

struct HttpRequest;
struct HttpResponse;

void HandleWebSettingsRequestGateway(
    const HttpRequest& req,
    const std::function<bool(const std::string&)>& isTokenValid,
    const std::function<bool(const HttpRequest&, const std::string&, HttpResponse&)>& handleApiRoute,
    const std::function<bool(const HttpRequest&, HttpResponse&)>& handleStaticAssetRoute,
    HttpResponse& resp);

} // namespace mousefx
