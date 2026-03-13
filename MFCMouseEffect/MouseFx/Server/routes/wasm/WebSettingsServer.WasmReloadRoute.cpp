#include "pch.h"
#include "WebSettingsServer.WasmReloadRoute.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRouteUtils.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {
using websettings_wasm_routes::BuildWasmActionResponse;
using websettings_wasm_routes::SetJsonResponse;

namespace {

constexpr const char* kErrorCodeNoController = "no_controller";
constexpr const char* kErrorCodeWasmHostUnavailable = "wasm_host_unavailable";
constexpr const char* kErrorCodeReloadFailed = "reload_failed";
constexpr const char* kErrorCodeReloadTargetMissing = "reload_target_missing";

} // namespace

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
    std::string errorCode = kErrorCodeNoController;
    if (controller && controller->WasmHost()) {
        error.clear();
        errorCode.clear();
        const wasm::HostDiagnostics beforeReload = controller->WasmHost()->Diagnostics();
        const bool hasReloadTarget =
            !beforeReload.activeManifestPath.empty() || !beforeReload.activeWasmPath.empty();
        controller->HandleCommand("{\"cmd\":\"wasm_reload\"}");
        const wasm::HostDiagnostics& afterReload = controller->WasmHost()->Diagnostics();
        ok = afterReload.lastError.empty();
        if (!ok) {
            error = afterReload.lastError;
            if (!hasReloadTarget) {
                errorCode = kErrorCodeReloadTargetMissing;
            } else {
                errorCode = TrimAscii(afterReload.lastLoadFailureCode);
                if (errorCode.empty()) {
                    errorCode = kErrorCodeReloadFailed;
                }
            }
        }
    } else if (controller) {
        error = "wasm host unavailable";
        errorCode = kErrorCodeWasmHostUnavailable;
    }
    SetJsonResponse(resp, BuildWasmActionResponse(controller, ok, error, errorCode).dump());
    return true;
}

} // namespace mousefx
