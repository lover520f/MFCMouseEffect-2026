#include "pch.h"
#include "WebSettingsServer.WasmRuntimeToggleRoutes.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"

namespace mousefx {
using websettings_wasm_routes::BuildWasmResponse;
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmRuntimeToggleApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/wasm/enable") {
        if (controller) {
            controller->HandleCommand("{\"cmd\":\"wasm_enable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/disable") {
        if (controller) {
            controller->HandleCommand("{\"cmd\":\"wasm_disable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller, true).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
