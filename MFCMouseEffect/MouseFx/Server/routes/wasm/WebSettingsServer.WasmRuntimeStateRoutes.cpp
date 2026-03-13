#include "pch.h"
#include "WebSettingsServer.WasmRuntimeStateRoutes.h"

#include <string>

#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRuntimePolicyRoute.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRuntimeToggleRoutes.h"

namespace mousefx {

bool HandleWebSettingsWasmRuntimeStateApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsWasmRuntimeToggleApiRoute(req, path, controller, resp)) {
        return true;
    }
    return HandleWebSettingsWasmRuntimePolicyApiRoute(req, path, controller, resp);
}

} // namespace mousefx
