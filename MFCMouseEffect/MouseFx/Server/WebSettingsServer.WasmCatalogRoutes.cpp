#include "pch.h"
#include "WebSettingsServer.WasmCatalogRoutes.h"

#include <string>

#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmCatalogQueryRoutes.h"
#include "MouseFx/Server/WebSettingsServer.WasmExportRoutes.h"
#include "MouseFx/Server/WebSettingsServer.WasmImportRoutes.h"

namespace mousefx {

bool HandleWebSettingsWasmCatalogApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsWasmCatalogQueryApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsWasmImportApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsWasmExportApiRoute(req, path, controller, resp)) {
        return true;
    }

    return false;
}

} // namespace mousefx
