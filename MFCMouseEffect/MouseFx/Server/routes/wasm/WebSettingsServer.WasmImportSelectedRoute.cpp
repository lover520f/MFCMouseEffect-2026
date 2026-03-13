#include "pch.h"
#include "WebSettingsServer.WasmImportSelectedRoute.h"

#include <string>

#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRouteUtils.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::ParseManifestPathUtf8;
using websettings_wasm_routes::ParseObjectOrEmpty;
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmImportSelectedRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    (void)controller;
    if (req.method != "POST" || path != "/api/wasm/import-selected") {
        return false;
    }

    const json payload = ParseObjectOrEmpty(req.body);
    const std::string manifestPathUtf8 = ParseManifestPathUtf8(payload);
    wasm::WasmPluginTransferService transfer;
    const wasm::PluginImportResult result = transfer.ImportFromManifestPath(Utf8ToWString(manifestPathUtf8));
    SetJsonResponse(resp, json({
        {"ok", result.ok},
        {"error", result.error},
        {"error_code", result.errorCode},
        {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
        {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
        {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
    }).dump());
    return true;
}

} // namespace mousefx
