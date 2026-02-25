#include "pch.h"
#include "WebSettingsServer.WasmRuntimeActionRoutes.h"

#include <string>

#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmLoadManifestRoute.h"
#include "MouseFx/Server/WebSettingsServer.WasmReloadRoute.h"

namespace mousefx {

bool HandleWebSettingsWasmRuntimeActionApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsWasmReloadApiRoute(req, path, controller, resp)) {
        return true;
    }
    return HandleWebSettingsWasmLoadManifestApiRoute(req, path, controller, resp);
}

} // namespace mousefx
