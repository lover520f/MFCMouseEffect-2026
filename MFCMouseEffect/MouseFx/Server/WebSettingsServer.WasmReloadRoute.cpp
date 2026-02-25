#include "pch.h"
#include "WebSettingsServer.WasmReloadRoute.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"

namespace mousefx {
using websettings_wasm_routes::BuildWasmActionResponse;
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmReloadApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "POST" || path != "/api/wasm/reload") {
        return false;
    }

    bool ok = false;
    std::string error = "no controller";
    if (controller && controller->WasmHost()) {
        error.clear();
        controller->HandleCommand("{\"cmd\":\"wasm_reload\"}");
        ok = controller->WasmHost()->Diagnostics().lastError.empty();
        if (!ok) {
            error = controller->WasmHost()->Diagnostics().lastError;
        }
    } else if (controller) {
        error = "wasm host unavailable";
    }
    SetJsonResponse(resp, BuildWasmActionResponse(controller, ok, error).dump());
    return true;
}

} // namespace mousefx
