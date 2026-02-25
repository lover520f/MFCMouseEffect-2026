#include "pch.h"
#include "WebSettingsServer.TestAutomationInjectionApiRoutes.h"

#include <string>

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestAutomationRouteUtils.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_test_automation::BuildSelectedBindingJson;
using websettings_test_automation::IsAutomationInjectionTestApiEnabled;
using websettings_test_automation::IsAutomationScopeTestApiEnabled;
using websettings_test_automation::NormalizeMouseHistoryEntries;
using websettings_test_automation::ParseActionHistory;
using websettings_test_automation::ParseAutomationMappings;
using websettings_test_automation::ParseProcessBaseName;
using websettings_test_automation::ParseShortcutKeys;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool HandleWebSettingsTestAutomationInjectionApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/automation/test-match-and-inject") {
        if (!IsAutomationInjectionTestApiEnabled() || !IsAutomationScopeTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::vector<std::string> rawHistory = ParseActionHistory(payload);
        std::vector<std::string> normalizedHistory;
        const std::vector<automation_match::ActionHistoryEntry> history =
            NormalizeMouseHistoryEntries(rawHistory, &normalizedHistory);
        const std::vector<AutomationKeyBinding> mappings = ParseAutomationMappings(payload);

        std::string processBaseName = ParseProcessBaseName(payload);
        if (processBaseName.empty()) {
            processBaseName = controller->CurrentForegroundProcessBaseName();
        }
        const std::string normalizedProcess = automation_scope::NormalizeProcessName(processBaseName);

        const automation_match::BindingMatchResult match = automation_match::FindBestEnabledBinding(
            mappings,
            history,
            processBaseName,
            automation_match::ChainTimingLimit{},
            automation_ids::NormalizeMouseActionId);

        bool injected = false;
        if (match.binding != nullptr) {
            const std::string keys = TrimAscii(match.binding->keys);
            if (!keys.empty()) {
                injected = controller->InjectShortcutForTest(keys);
            }
        }

        const json selected = BuildSelectedBindingJson(match);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName},
            {"process_normalized", normalizedProcess},
            {"history", rawHistory},
            {"history_normalized", normalizedHistory},
            {"mapping_count", static_cast<uint64_t>(mappings.size())},
            {"matched", match.binding != nullptr},
            {"injected", injected},
            {"selected_binding_index", match.binding != nullptr ? static_cast<int64_t>(match.bindingIndex) : -1},
            {"selected_chain_length", static_cast<uint64_t>(match.chainLength)},
            {"selected_scope_specificity", match.scopeSpecificity},
            {"selected_keys", match.binding != nullptr ? match.binding->keys : std::string{}},
            {"selected", selected},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/test-inject-shortcut") {
        if (!IsAutomationInjectionTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string keys = ParseShortcutKeys(payload);
        const bool accepted = !keys.empty() && controller->InjectShortcutForTest(keys);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"keys", keys},
            {"accepted", accepted},
            {"process", controller->CurrentForegroundProcessBaseName()},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
