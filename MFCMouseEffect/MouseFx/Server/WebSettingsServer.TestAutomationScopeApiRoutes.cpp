#include "pch.h"
#include "WebSettingsServer.TestAutomationScopeApiRoutes.h"

#include <string>

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestAutomationRouteUtils.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_test_automation::BuildSelectedBindingJson;
using websettings_test_automation::IsAutomationScopeTestApiEnabled;
using websettings_test_automation::NormalizeMouseHistoryEntries;
using websettings_test_automation::ParseActionHistory;
using websettings_test_automation::ParseAppScopes;
using websettings_test_automation::ParseAutomationMappings;
using websettings_test_automation::ParseProcessBaseName;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool HandleWebSettingsTestAutomationScopeApiRoute(
    const HttpRequest& req,
    const std::string& path,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/automation/test-app-scope-match") {
        if (!IsAutomationScopeTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::vector<std::string> rawScopes = ParseAppScopes(payload);
        const std::string processBaseName = ParseProcessBaseName(payload);
        const std::string normalizedProcess = automation_scope::NormalizeProcessName(processBaseName);

        json normalizedScopes = json::array();
        for (const std::string& scope : rawScopes) {
            normalizedScopes.push_back(automation_scope::NormalizeScopeToken(scope));
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName},
            {"process_normalized", normalizedProcess},
            {"app_scopes", rawScopes},
            {"app_scopes_normalized", normalizedScopes},
            {"matched", automation_scope::AppScopeMatchesProcess(rawScopes, processBaseName)},
            {"specificity", automation_scope::AppScopeSpecificity(rawScopes)},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/test-binding-priority") {
        if (!IsAutomationScopeTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::vector<std::string> rawHistory = ParseActionHistory(payload);
        std::vector<std::string> normalizedHistory;
        const std::vector<automation_match::ActionHistoryEntry> history =
            NormalizeMouseHistoryEntries(rawHistory, &normalizedHistory);
        const std::vector<AutomationKeyBinding> mappings = ParseAutomationMappings(payload);
        const std::string processBaseName = ParseProcessBaseName(payload);
        const std::string normalizedProcess = automation_scope::NormalizeProcessName(processBaseName);
        const automation_match::BindingMatchResult match = automation_match::FindBestEnabledBinding(
            mappings,
            history,
            processBaseName,
            automation_match::ChainTimingLimit{},
            automation_ids::NormalizeMouseActionId);

        const json selected = BuildSelectedBindingJson(match);

        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName},
            {"process_normalized", normalizedProcess},
            {"history", rawHistory},
            {"history_normalized", normalizedHistory},
            {"mapping_count", static_cast<uint64_t>(mappings.size())},
            {"matched", match.binding != nullptr},
            {"selected_binding_index", match.binding != nullptr ? static_cast<int64_t>(match.bindingIndex) : -1},
            {"selected_chain_length", static_cast<uint64_t>(match.chainLength)},
            {"selected_scope_specificity", match.scopeSpecificity},
            {"selected_keys", match.binding != nullptr ? match.binding->keys : std::string{}},
            {"selected", selected},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
