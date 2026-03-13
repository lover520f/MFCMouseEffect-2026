#include "pch.h"
#include "WebSettingsServer.AutomationCatalogRoutes.h"

#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/automation/WebSettingsServer.AutomationRouteUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_automation_routes::LoadAutomationAppCatalog;
using websettings_automation_routes::ParseForceRefresh;
using websettings_automation_routes::ParseObjectOrEmpty;
using websettings_automation_routes::SetJsonResponse;

bool HandleWebSettingsAutomationCatalogApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/automation/active-process") {
        const std::string processBaseName = controller
            ? controller->CurrentForegroundProcessBaseName()
            : std::string{};
        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/app-catalog") {
        const json payload = ParseObjectOrEmpty(req.body);
        const bool forceRefresh = ParseForceRefresh(payload);
        const std::vector<platform::ApplicationCatalogEntry> entries = LoadAutomationAppCatalog(forceRefresh);

        json apps = json::array();
        for (const auto& entry : entries) {
            apps.push_back({
                {"exe", entry.processName},
                {"label", entry.displayName},
                {"source", entry.source},
            });
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"apps", apps},
            {"count", apps.size()},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
