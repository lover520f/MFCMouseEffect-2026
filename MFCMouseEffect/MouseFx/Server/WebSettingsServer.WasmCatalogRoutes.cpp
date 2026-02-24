#include "pch.h"
#include "WebSettingsServer.WasmCatalogRoutes.h"

#include <filesystem>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmPluginCatalog.h"
#include "MouseFx/Core/Wasm/WasmPluginPaths.h"
#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"
#include "Platform/PlatformNativeFolderPicker.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::ParseInitialPathUtf8;
using websettings_wasm_routes::ParseManifestPathUtf8;
using websettings_wasm_routes::ParseObjectOrEmpty;
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmCatalogApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/wasm/catalog") {
        wasm::WasmPluginCatalog catalog;
        std::wstring configuredCatalogRoot;
        if (controller) {
            configuredCatalogRoot = Utf8ToWString(controller->GetConfigSnapshot().wasm.catalogRootPath);
        }
        const std::vector<std::wstring> searchRoots = wasm::WasmPluginPaths::ResolveSearchRoots(configuredCatalogRoot);
        const wasm::PluginCatalogResult result = catalog.DiscoverFromRoots(searchRoots);

        json plugins = json::array();
        for (const auto& plugin : result.plugins) {
            plugins.push_back({
                {"id", plugin.manifest.id},
                {"name", plugin.manifest.name},
                {"version", plugin.manifest.version},
                {"api_version", plugin.manifest.apiVersion},
                {"manifest_path", Utf16ToUtf8(plugin.manifestPath.c_str())},
                {"wasm_path", Utf16ToUtf8(plugin.wasmPath.c_str())},
            });
        }

        json errors = json::array();
        for (const auto& error : result.errors) {
            errors.push_back(error);
        }

        json roots = json::array();
        for (const std::wstring& root : searchRoots) {
            roots.push_back(Utf16ToUtf8(root.c_str()));
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"plugins", plugins},
            {"errors", errors},
            {"search_roots", roots},
            {"count", plugins.size()},
            {"error_count", errors.size()},
        }).dump());
        return true;
    }

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

    if (req.method == "POST" && path == "/api/wasm/import-from-folder-dialog") {
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
            return true;
        }

        const std::filesystem::path pluginDir(picked.folderPath);
        const std::filesystem::path manifestPath = pluginDir / L"plugin.json";
        std::error_code ec;
        if (!std::filesystem::exists(manifestPath, ec) || ec || !std::filesystem::is_regular_file(manifestPath, ec) || ec) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"cancelled", false},
                {"error", "plugin.json is missing in selected folder"},
                {"selected_folder_path", Utf16ToUtf8(pluginDir.wstring().c_str())},
            }).dump());
            return true;
        }

        wasm::WasmPluginTransferService transfer;
        const wasm::PluginImportResult result = transfer.ImportFromManifestPath(manifestPath.wstring());
        SetJsonResponse(resp, json({
            {"ok", result.ok},
            {"cancelled", false},
            {"error", result.error},
            {"selected_folder_path", Utf16ToUtf8(pluginDir.wstring().c_str())},
            {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
            {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
            {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/export-all") {
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
            {"export_path", Utf16ToUtf8(result.exportDirectoryPath.c_str())},
            {"count", result.exportedPluginCount},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
