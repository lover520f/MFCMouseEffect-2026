#include "pch.h"
#include "WebSettingsServer.WasmExportRoutes.h"

#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmPluginPaths.h"
#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmExportApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "POST" || path != "/api/wasm/export-all") {
        return false;
    }

    std::wstring configuredCatalogRoot;
    if (controller) {
        configuredCatalogRoot = Utf8ToWString(controller->GetConfigSnapshot().wasm.catalogRootPath);
    }
    const std::vector<std::wstring> searchRoots = wasm::WasmPluginPaths::ResolveSearchRoots(configuredCatalogRoot);

    wasm::WasmPluginTransferService transfer;
    const wasm::PluginExportResult result = transfer.ExportAllDiscoveredPlugins(searchRoots);
    SetJsonResponse(resp, json({
        {"ok", result.ok},
        {"error", result.error},
        {"error_code", result.errorCode},
        {"export_path", Utf16ToUtf8(result.exportDirectoryPath.c_str())},
        {"count", result.exportedPluginCount},
    }).dump());
    return true;
}

} // namespace mousefx
