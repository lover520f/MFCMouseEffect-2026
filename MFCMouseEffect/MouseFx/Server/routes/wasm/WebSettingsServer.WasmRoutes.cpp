#include "pch.h"
#include "WebSettingsServer.WasmRoutes.h"

#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmCatalogRoutes.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRuntimeRoutes.h"

namespace mousefx {

bool HandleWebSettingsWasmApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsWasmCatalogApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsWasmRuntimeApiRoute(req, path, controller, resp)) {
        return true;
    }

    return false;
}

} // namespace mousefx
