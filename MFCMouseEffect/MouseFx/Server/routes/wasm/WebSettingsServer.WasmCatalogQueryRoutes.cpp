#include "pch.h"
#include "WebSettingsServer.WasmCatalogQueryRoutes.h"

#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmPluginCatalog.h"
#include "MouseFx/Core/Wasm/WasmPluginPaths.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRouteUtils.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::SetJsonResponse;
using websettings_wasm_routes::ParseObjectOrEmpty;

namespace {

json InputKindsToJson(uint32_t mask) {
    json kinds = json::array();
    if (mask & wasm::kManifestInputKindClickBit) {
        kinds.push_back("click");
    }
    if (mask & wasm::kManifestInputKindMoveBit) {
        kinds.push_back("move");
    }
    if (mask & wasm::kManifestInputKindScrollBit) {
        kinds.push_back("scroll");
    }
    if (mask & wasm::kManifestInputKindHoldStartBit) {
        kinds.push_back("hold_start");
    }
    if (mask & wasm::kManifestInputKindHoldUpdateBit) {
        kinds.push_back("hold_update");
    }
    if (mask & wasm::kManifestInputKindHoldEndBit) {
        kinds.push_back("hold_end");
    }
    if (mask & wasm::kManifestInputKindHoverStartBit) {
        kinds.push_back("hover_start");
    }
    if (mask & wasm::kManifestInputKindHoverEndBit) {
        kinds.push_back("hover_end");
    }
    if (mask & wasm::kManifestInputKindIndicatorClickBit) {
        kinds.push_back("indicator_click");
    }
    if (mask & wasm::kManifestInputKindIndicatorScrollBit) {
        kinds.push_back("indicator_scroll");
    }
    if (mask & wasm::kManifestInputKindIndicatorKeyBit) {
        kinds.push_back("indicator_key");
    }
    return kinds;
}

json SurfaceKindsToJson(uint32_t mask) {
    json surfaces = json::array();
    if (mask & wasm::kManifestSurfaceEffectsBit) {
        surfaces.push_back("effects");
    }
    if (mask & wasm::kManifestSurfaceIndicatorBit) {
        surfaces.push_back("indicator");
    }
    return surfaces;
}

bool SupportsCatalogSurface(const wasm::PluginManifest& manifest, const std::string& surfaceFilterRaw) {
    const std::string surfaceFilter = ToLowerAscii(TrimAscii(surfaceFilterRaw));
    if (surfaceFilter.empty()) {
        return true;
    }

    constexpr uint32_t kIndicatorInputMask =
        wasm::kManifestInputKindIndicatorClickBit |
        wasm::kManifestInputKindIndicatorScrollBit |
        wasm::kManifestInputKindIndicatorKeyBit;
    constexpr uint32_t kEffectsInputMask = wasm::kManifestInputKindAllBits & ~kIndicatorInputMask;

    if (surfaceFilter == "effects") {
        if (manifest.hasExplicitSurfaceKinds) {
            return (manifest.surfaceKindsMask & wasm::kManifestSurfaceEffectsBit) != 0u;
        }
        return (manifest.inputKindsMask & kEffectsInputMask) != 0u;
    }
    if (surfaceFilter == "indicator") {
        if (manifest.hasExplicitSurfaceKinds) {
            return (manifest.surfaceKindsMask & wasm::kManifestSurfaceIndicatorBit) != 0u;
        }
        return (manifest.inputKindsMask & kIndicatorInputMask) != 0u;
    }
    return true;
}

} // namespace

bool HandleWebSettingsWasmCatalogQueryApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "POST" || path != "/api/wasm/catalog") {
        return false;
    }

    wasm::WasmPluginCatalog catalog;
    const json payload = ParseObjectOrEmpty(req.body);
    std::string surfaceFilter;
    if (payload.contains("surface") && payload["surface"].is_string()) {
        surfaceFilter = TrimAscii(payload["surface"].get<std::string>());
    }
    std::wstring configuredCatalogRoot;
    if (controller) {
        configuredCatalogRoot = Utf8ToWString(controller->GetConfigSnapshot().wasm.catalogRootPath);
    }
    const std::vector<std::wstring> searchRoots = wasm::WasmPluginPaths::ResolveSearchRoots(configuredCatalogRoot);
    const wasm::PluginCatalogResult result = catalog.DiscoverFromRoots(searchRoots);

    json plugins = json::array();
    for (const auto& plugin : result.plugins) {
        if (!SupportsCatalogSurface(plugin.manifest, surfaceFilter)) {
            continue;
        }
        plugins.push_back({
            {"id", plugin.manifest.id},
            {"name", plugin.manifest.name},
            {"version", plugin.manifest.version},
            {"api_version", plugin.manifest.apiVersion},
            {"input_kinds", InputKindsToJson(plugin.manifest.inputKindsMask)},
            {"surfaces", SurfaceKindsToJson(plugin.manifest.surfaceKindsMask)},
            {"has_explicit_surfaces", plugin.manifest.hasExplicitSurfaceKinds},
            {"enable_frame_tick", plugin.manifest.enableFrameTick},
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
        {"surface_filter", surfaceFilter},
        {"count", plugins.size()},
        {"error_count", errors.size()},
    }).dump());
    return true;
}

} // namespace mousefx
