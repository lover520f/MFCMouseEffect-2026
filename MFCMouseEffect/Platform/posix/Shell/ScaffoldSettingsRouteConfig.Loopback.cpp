#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.Internal.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteCodec.h"

#include <string>
#include <string_view>

namespace mousefx::platform::scaffold {

void ParseLoopbackUrlRoute(std::string_view url, SettingsRoute* route) {
    if (!route) {
        return;
    }

    constexpr std::string_view kPrefix = "http://127.0.0.1:";
    if (url.rfind(kPrefix, 0) != 0) {
        route->useEmbeddedServer = false;
        return;
    }

    const size_t portStart = kPrefix.size();
    const size_t slashPos = url.find('/', portStart);
    if (slashPos == std::string_view::npos) {
        route->useEmbeddedServer = false;
        return;
    }

    uint16_t parsedPort = 0;
    if (!TryParseUnsignedPort(url.substr(portStart, slashPos - portStart), &parsedPort)) {
        route->useEmbeddedServer = false;
        return;
    }

    route->port = parsedPort;
    const std::string pathWithQuery(url.substr(slashPos));
    route->path = NormalizePath(PathWithoutQuery(pathWithQuery));

    const std::string token = QueryValue(pathWithQuery, "token");
    if (!token.empty()) {
        route->token = token;
    }
}

} // namespace mousefx::platform::scaffold
