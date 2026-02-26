#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.Internal.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteCodec.h"

#include <cstdlib>

namespace mousefx::platform::scaffold {

SettingsRoute BuildSettingsRoute() {
    SettingsRoute route{};
    route.url = "http://127.0.0.1:9527/?token=scaffold";
    route.path = "/";

    const char* env = std::getenv("MFX_SCAFFOLD_SETTINGS_URL");
    if (!env || *env == '\0') {
        return route;
    }

    const std::string url(env);
    if (!IsSettingsUrlShellSafe(url)) {
        return route;
    }

    route.url = url;
    ParseLoopbackUrlRoute(url, &route);
    return route;
}

} // namespace mousefx::platform::scaffold
