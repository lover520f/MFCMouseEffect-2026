#include "pch.h"
#include "WebSettingsServer.WasmRuntimeStateRoutes.h"

#include <string>

#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRuntimePolicyRoute.h"
#include "MouseFx/Server/WebSettingsServer.WasmRuntimeToggleRoutes.h"

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
