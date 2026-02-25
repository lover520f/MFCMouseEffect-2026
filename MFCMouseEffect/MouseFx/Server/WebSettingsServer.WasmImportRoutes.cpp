#include "pch.h"
#include "WebSettingsServer.WasmImportRoutes.h"

#include <filesystem>
#include <string>

#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformNativeFolderPicker.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::ParseInitialPathUtf8;
using websettings_wasm_routes::ParseManifestPathUtf8;
using websettings_wasm_routes::ParseObjectOrEmpty;
using websettings_wasm_routes::SetJsonResponse;

namespace {

void SetFolderImportMissingManifestResponse(const std::wstring& folderPath, HttpResponse& resp) {
    SetJsonResponse(resp, json({
        {"ok", false},
        {"cancelled", false},
        {"error", "plugin.json is missing in selected folder"},
        {"selected_folder_path", Utf16ToUtf8(folderPath.c_str())},
    }).dump());
}

void SetFolderImportCancelledResponse(
    const platform::NativeFolderPickResult& picked,
    const json& payload,
    HttpResponse& resp) {
    std::string selectedFolderPath = Utf16ToUtf8(picked.folderPath.c_str());
    if (selectedFolderPath.empty()) {
        selectedFolderPath = ParseInitialPathUtf8(payload);
    }
    SetJsonResponse(resp, json({
        {"ok", false},
        {"cancelled", picked.cancelled},
        {"error", picked.error},
        {"selected_folder_path", selectedFolderPath},
    }).dump());
}

void SetFolderImportResultResponse(
    const std::wstring& folderPath,
    const wasm::PluginImportResult& result,
    HttpResponse& resp) {
    SetJsonResponse(resp, json({
        {"ok", result.ok},
        {"cancelled", false},
        {"error", result.error},
        {"selected_folder_path", Utf16ToUtf8(folderPath.c_str())},
        {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
        {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
        {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
    }).dump());
}

} // namespace

bool HandleWebSettingsWasmImportApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    (void)controller;

    if (req.method == "POST" && path == "/api/wasm/import-selected") {
        const json payload = ParseObjectOrEmpty(req.body);
        const std::string manifestPathUtf8 = ParseManifestPathUtf8(payload);
        wasm::WasmPluginTransferService transfer;
        const wasm::PluginImportResult result = transfer.ImportFromManifestPath(Utf8ToWString(manifestPathUtf8));
        SetJsonResponse(resp, json({
            {"ok", result.ok},
            {"error", result.error},
            {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
            {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
            {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
        }).dump());
        return true;
    }

    if (req.method != "POST" || path != "/api/wasm/import-from-folder-dialog") {
        return false;
    }

    const json payload = ParseObjectOrEmpty(req.body);
    if (payload.contains("probe_only") && payload["probe_only"].is_boolean() && payload["probe_only"].get<bool>()) {
        const bool supported = platform::IsNativeFolderPickerSupported();
        SetJsonResponse(resp, json({
            {"ok", true},
            {"probe_only", true},
            {"supported", supported},
            {"cancelled", false},
            {"error", supported ? "" : "native_folder_picker_not_supported"},
            {"selected_folder_path", ParseInitialPathUtf8(payload)},
            {"source_manifest_path", ""},
            {"manifest_path", ""},
            {"primary_root_path", ""},
        }).dump());
        return true;
    }

    const std::wstring initialPath = Utf8ToWString(ParseInitialPathUtf8(payload));
    const platform::NativeFolderPickResult picked = platform::PickFolder(
        L"Select WASM plugin folder",
        initialPath);

    if (!picked.ok) {
        SetFolderImportCancelledResponse(picked, payload, resp);
        return true;
    }

    const std::filesystem::path pluginDir(picked.folderPath);
    const std::filesystem::path manifestPath = pluginDir / L"plugin.json";
    std::error_code ec;
    if (!std::filesystem::exists(manifestPath, ec) ||
        ec ||
        !std::filesystem::is_regular_file(manifestPath, ec) ||
        ec) {
        SetFolderImportMissingManifestResponse(pluginDir.wstring(), resp);
        return true;
    }

    wasm::WasmPluginTransferService transfer;
    const wasm::PluginImportResult result = transfer.ImportFromManifestPath(manifestPath.wstring());
    SetFolderImportResultResponse(pluginDir.wstring(), result, resp);
    return true;
}

} // namespace mousefx
