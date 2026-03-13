#pragma once

#include <string>
#include <vector>

#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx::websettings_test_automation {

bool IsAutomationScopeTestApiEnabled();
bool IsAutomationShortcutTestApiEnabled();
bool IsAutomationInjectionTestApiEnabled();

std::vector<std::string> ParseAppScopes(const nlohmann::json& payload);
std::string ParseProcessBaseName(const nlohmann::json& payload);
std::string ParseShortcutKeys(const nlohmann::json& payload);
std::vector<std::string> ParseActionHistory(const nlohmann::json& payload);
std::vector<AutomationKeyBinding> ParseAutomationMappings(const nlohmann::json& payload);
std::vector<automation_match::ActionHistoryEntry> NormalizeMouseHistoryEntries(
    const std::vector<std::string>& actions,
    std::vector<std::string>* normalizedOut);
nlohmann::json BuildSelectedBindingJson(const automation_match::BindingMatchResult& match);

} // namespace mousefx::websettings_test_automation
