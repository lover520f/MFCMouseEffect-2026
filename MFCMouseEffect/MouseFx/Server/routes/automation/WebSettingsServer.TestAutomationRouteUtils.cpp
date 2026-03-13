#include "pch.h"
#include "WebSettingsServer.TestAutomationRouteUtils.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestRouteCommon.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx::websettings_test_automation {
namespace {

using websettings_test_routes::IsEnabledByEnv;
using websettings_test_routes::ParseBooleanOrDefault;

std::string ParseStringOrDefault(const json& payload, const char* key, std::string defaultValue = {}) {
    if (!payload.contains(key) || !payload[key].is_string()) {
        return defaultValue;
    }
    return payload[key].get<std::string>();
}

} // namespace

bool IsAutomationScopeTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_AUTOMATION_SCOPE_TEST_API");
}

bool IsAutomationShortcutTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API");
}

bool IsAutomationInjectionTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_AUTOMATION_INJECTION_TEST_API");
}

std::vector<std::string> ParseAppScopes(const json& payload) {
    std::vector<std::string> out;
    if (payload.contains("app_scopes") && payload["app_scopes"].is_array()) {
        for (const auto& item : payload["app_scopes"]) {
            if (item.is_string()) {
                out.push_back(item.get<std::string>());
            }
        }
    }
    if (out.empty() && payload.contains("app_scope") && payload["app_scope"].is_string()) {
        out.push_back(payload["app_scope"].get<std::string>());
    }
    return out;
}

std::string ParseProcessBaseName(const json& payload) {
    if (!payload.contains("process") || !payload["process"].is_string()) {
        return {};
    }
    return payload["process"].get<std::string>();
}

std::string ParseShortcutKeys(const json& payload) {
    if (!payload.contains("keys") || !payload["keys"].is_string()) {
        return {};
    }
    return TrimAscii(payload["keys"].get<std::string>());
}

std::vector<std::string> ParseActionHistory(const json& payload) {
    std::vector<std::string> actions;
    if (payload.contains("history") && payload["history"].is_array()) {
        for (const auto& item : payload["history"]) {
            if (item.is_string()) {
                actions.push_back(item.get<std::string>());
            }
        }
    }
    if (actions.empty() && payload.contains("action") && payload["action"].is_string()) {
        actions.push_back(payload["action"].get<std::string>());
    }
    return actions;
}

std::vector<AutomationKeyBinding> ParseAutomationMappings(const json& payload) {
    std::vector<AutomationKeyBinding> mappings;
    if (!payload.contains("mappings") || !payload["mappings"].is_array()) {
        return mappings;
    }

    for (const auto& item : payload["mappings"]) {
        if (!item.is_object()) {
            continue;
        }

        AutomationKeyBinding binding{};
        binding.enabled = ParseBooleanOrDefault(item, "enabled", true);
        binding.trigger = TrimAscii(ParseStringOrDefault(item, "trigger"));
        binding.keys = TrimAscii(ParseStringOrDefault(item, "keys"));
        binding.appScopes = ParseAppScopes(item);
        mappings.push_back(std::move(binding));
    }

    return mappings;
}

std::vector<automation_match::ActionHistoryEntry> NormalizeMouseHistoryEntries(
    const std::vector<std::string>& actions,
    std::vector<std::string>* normalizedOut) {
    std::vector<automation_match::ActionHistoryEntry> history;
    if (normalizedOut) {
        normalizedOut->clear();
    }

    const auto baseTime = std::chrono::steady_clock::now();
    for (size_t i = 0; i < actions.size(); ++i) {
        std::string normalized = automation_ids::NormalizeMouseActionId(actions[i]);
        if (normalized.empty()) {
            continue;
        }
        if (normalizedOut) {
            normalizedOut->push_back(normalized);
        }
        history.push_back(automation_match::ActionHistoryEntry{
            std::move(normalized),
            baseTime + std::chrono::milliseconds(static_cast<int64_t>(i * 10)),
        });
    }

    return history;
}

json BuildSelectedBindingJson(const automation_match::BindingMatchResult& match) {
    if (match.binding == nullptr) {
        return nullptr;
    }

    json normalizedScopes = json::array();
    for (const auto& scope : match.binding->appScopes) {
        normalizedScopes.push_back(automation_scope::NormalizeScopeToken(scope));
    }

    return json({
        {"index", static_cast<uint64_t>(match.bindingIndex)},
        {"trigger", match.binding->trigger},
        {"keys", match.binding->keys},
        {"app_scopes", match.binding->appScopes},
        {"app_scopes_normalized", normalizedScopes},
        {"chain_length", static_cast<uint64_t>(match.chainLength)},
        {"scope_specificity", match.scopeSpecificity},
    });
}

} // namespace mousefx::websettings_test_automation
