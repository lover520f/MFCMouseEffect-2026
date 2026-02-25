#include "pch.h"
#include "WebSettingsServer.WasmLoadManifestRoute.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::BuildWasmActionResponse;
using websettings_wasm_routes::IsSameManifestPath;
using websettings_wasm_routes::ParseManifestPathUtf8;
using websettings_wasm_routes::ParseObjectOrEmpty;
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmLoadManifestApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "POST" || path != "/api/wasm/load-manifest") {
        return false;
    }

    bool ok = false;
    std::string error = "no controller";
    if (controller && controller->WasmHost()) {
        error.clear();
        const json payload = ParseObjectOrEmpty(req.body);
        const std::string manifestPathUtf8 = ParseManifestPathUtf8(payload);
        if (!manifestPathUtf8.empty()) {
            json cmd;
            cmd["cmd"] = "wasm_load_manifest";
            cmd["manifest_path"] = manifestPathUtf8;
            controller->HandleCommand(cmd.dump());
            const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
            ok = diag.pluginLoaded &&
                IsSameManifestPath(Utf8ToWString(manifestPathUtf8), diag.activeManifestPath);
            if (!ok) {
                error = diag.lastError;
                if (error.empty()) {
                    error = "manifest switch did not take effect";
                }
            }
        } else {
            error = "manifest_path required";
        }
    } else if (controller) {
        error = "wasm host unavailable";
    }
    SetJsonResponse(resp, BuildWasmActionResponse(controller, ok, error).dump());
    return true;
}

} // namespace mousefx
