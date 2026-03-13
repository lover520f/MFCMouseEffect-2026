#include "pch.h"
#include "WebSettingsServer.WasmRuntimeActionRoutes.h"

#include <string>

#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmLoadManifestRoute.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmReloadRoute.h"

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
