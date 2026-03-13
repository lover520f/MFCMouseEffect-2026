#include "pch.h"
#include "WebSettingsServer.WasmImportRoutes.h"

#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmImportFolderDialogRoute.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmImportSelectedRoute.h"

namespace mousefx {

bool HandleWebSettingsWasmImportApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsWasmImportSelectedRoute(req, path, controller, resp)) {
        return true;
    }
    return HandleWebSettingsWasmImportFolderDialogRoute(req, path, controller, resp);
}

} // namespace mousefx
