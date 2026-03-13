#include "pch.h"
#include "WebSettingsServer.WasmRuntimeRoutes.h"

#include <string>

#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRuntimeActionRoutes.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRuntimeStateRoutes.h"

namespace mousefx {

bool HandleWebSettingsWasmRuntimeApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsWasmRuntimeStateApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsWasmRuntimeActionApiRoute(req, path, controller, resp)) {
        return true;
    }

    return false;
}

} // namespace mousefx
