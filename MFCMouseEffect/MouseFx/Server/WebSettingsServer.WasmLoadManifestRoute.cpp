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

namespace {

constexpr const char* kErrorCodeNoController = "no_controller";
constexpr const char* kErrorCodeWasmHostUnavailable = "wasm_host_unavailable";
constexpr const char* kErrorCodeManifestPathRequired = "manifest_path_required";
constexpr const char* kErrorCodeLoadManifestFailed = "load_manifest_failed";

bool IsIndicatorSurface(const std::string& surface) {
    return ToLowerAscii(TrimAscii(surface)) == "indicator";
}

} // namespace

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
    std::string errorCode = kErrorCodeNoController;
    if (controller) {
        error.clear();
        errorCode.clear();
        const json payload = ParseObjectOrEmpty(req.body);
        const std::string manifestPathUtf8 = ParseManifestPathUtf8(payload);
        std::string surface;
        std::string effectChannel;
        if (payload.contains("surface") && payload["surface"].is_string()) {
            surface = TrimAscii(payload["surface"].get<std::string>());
        }
        if (payload.contains("effect_channel") && payload["effect_channel"].is_string()) {
            effectChannel = TrimAscii(payload["effect_channel"].get<std::string>());
        }
        wasm::WasmEffectHost* host = nullptr;
        if (IsIndicatorSurface(surface)) {
            host = controller->WasmHostForSurface(surface);
        } else {
            host = controller->WasmEffectsHostForChannel(effectChannel);
        }
        if (!host) {
            error = "wasm host unavailable";
            errorCode = kErrorCodeWasmHostUnavailable;
            SetJsonResponse(resp, BuildWasmActionResponse(controller, false, error, errorCode).dump());
            return true;
        }
        if (!manifestPathUtf8.empty()) {
            json cmd;
            cmd["cmd"] = "wasm_load_manifest";
            cmd["manifest_path"] = manifestPathUtf8;
            if (!surface.empty()) {
                cmd["surface"] = surface;
            }
            if (!effectChannel.empty()) {
                cmd["effect_channel"] = effectChannel;
            }
            controller->HandleCommand(cmd.dump());
            const wasm::HostDiagnostics& diag = host->Diagnostics();
            ok = diag.pluginLoaded &&
                IsSameManifestPath(Utf8ToWString(manifestPathUtf8), diag.activeManifestPath);
            if (!ok) {
                error = diag.lastError;
                if (error.empty()) {
                    error = "manifest switch did not take effect";
                }
                errorCode = TrimAscii(diag.lastLoadFailureCode);
                if (errorCode.empty()) {
                    errorCode = kErrorCodeLoadManifestFailed;
                }
            }
        } else {
            error = "manifest_path required";
            errorCode = kErrorCodeManifestPathRequired;
        }
    }
    SetJsonResponse(resp, BuildWasmActionResponse(controller, ok, error, errorCode).dump());
    return true;
}

} // namespace mousefx
