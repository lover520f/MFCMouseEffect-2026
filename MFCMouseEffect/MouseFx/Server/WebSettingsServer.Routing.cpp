#include "pch.h"
#include "WebSettingsServer.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <cwctype>
#include <exception>
#include <filesystem>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Automation/ShortcutTextFormatter.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmEventInvokeExecutor.h"
#include "MouseFx/Core/Wasm/WasmPluginCatalog.h"
#include "MouseFx/Core/Wasm/WasmPluginPaths.h"
#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/SettingsSchemaBuilder.h"
#include "MouseFx/Server/SettingsStateMapper.h"
#include "MouseFx/Server/WebUiAssets.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformApplicationCatalog.h"
#include "Platform/PlatformNativeFolderPicker.h"
#include "Platform/macos/System/MacosVirtualKeyMapper.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

std::string StripQueryString(const std::string& path) {
    const size_t queryPos = path.find('?');
    if (queryPos == std::string::npos) {
        return path;
    }
    return path.substr(0, queryPos);
}

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = body;
}

void SetPlainResponse(HttpResponse& resp, int code, const std::string& body) {
    resp.statusCode = code;
    resp.contentType = "text/plain; charset=utf-8";
    resp.body = body;
}

json ParseObjectOrEmpty(const std::string& body) {
    if (body.empty()) {
        return json::object();
    }
    try {
        json parsed = json::parse(body);
        if (parsed.is_object()) {
            return parsed;
        }
    } catch (...) {
    }
    return json::object();
}

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLowerAscii(lhs[i]) != ToLowerAscii(rhs[i])) {
            return false;
        }
    }
    return true;
}

std::string ParseSessionId(const json& payload) {
    if (!payload.contains("session") || !payload["session"].is_string()) {
        return {};
    }
    return payload["session"].get<std::string>();
}

std::string PollStateToText(ShortcutCaptureSession::PollState state) {
    switch (state) {
    case ShortcutCaptureSession::PollState::Pending:
        return "pending";
    case ShortcutCaptureSession::PollState::Captured:
        return "captured";
    case ShortcutCaptureSession::PollState::Expired:
        return "expired";
    case ShortcutCaptureSession::PollState::InvalidSession:
    default:
        return "invalid";
    }
}

bool ParseForceRefresh(const json& payload) {
    if (!payload.contains("force")) {
        return false;
    }
    if (payload["force"].is_boolean()) {
        return payload["force"].get<bool>();
    }
    if (payload["force"].is_number_integer()) {
        return payload["force"].get<int>() != 0;
    }
    return false;
}

bool ParseBooleanOrDefault(const json& payload, const char* key, bool defaultValue) {
    if (!payload.contains(key)) {
        return defaultValue;
    }
    if (payload[key].is_boolean()) {
        return payload[key].get<bool>();
    }
    if (payload[key].is_number_integer()) {
        return payload[key].get<int>() != 0;
    }
    return defaultValue;
}

bool IsWasmTestDispatchApiEnabled() {
    const char* raw = std::getenv("MFX_ENABLE_WASM_TEST_DISPATCH_API");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }

    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

bool IsAutomationScopeTestApiEnabled() {
    const char* raw = std::getenv("MFX_ENABLE_AUTOMATION_SCOPE_TEST_API");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }

    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

bool IsAutomationShortcutTestApiEnabled() {
    const char* raw = std::getenv("MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }

    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

bool IsAutomationInjectionTestApiEnabled() {
    const char* raw = std::getenv("MFX_ENABLE_AUTOMATION_INJECTION_TEST_API");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }

    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

bool IsInputIndicatorTestApiEnabled() {
    const char* raw = std::getenv("MFX_ENABLE_INPUT_INDICATOR_TEST_API");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }

    const std::string_view value(raw);
    return value == "1" ||
           EqualsIgnoreCaseAscii(value, "true") ||
           EqualsIgnoreCaseAscii(value, "yes") ||
           EqualsIgnoreCaseAscii(value, "on");
}

int32_t ParseInt32OrDefault(const json& payload, const char* key, int32_t defaultValue) {
    if (!payload.contains(key) || !payload[key].is_number_integer()) {
        return defaultValue;
    }
    const int64_t raw = payload[key].get<int64_t>();
    if (raw < static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
        return std::numeric_limits<int32_t>::min();
    }
    if (raw > static_cast<int64_t>(std::numeric_limits<int32_t>::max())) {
        return std::numeric_limits<int32_t>::max();
    }
    return static_cast<int32_t>(raw);
}

uint8_t ParseButtonOrDefault(const json& payload, const char* key, uint8_t defaultValue) {
    if (!payload.contains(key) || !payload[key].is_number_integer()) {
        return defaultValue;
    }
    const int64_t raw = payload[key].get<int64_t>();
    if (raw <= 0) {
        return 0;
    }
    if (raw > static_cast<int64_t>(std::numeric_limits<uint8_t>::max())) {
        return std::numeric_limits<uint8_t>::max();
    }
    return static_cast<uint8_t>(raw);
}

std::string ParseManifestPathUtf8(const json& payload) {
    if (!payload.contains("manifest_path") || !payload["manifest_path"].is_string()) {
        return {};
    }
    return payload["manifest_path"].get<std::string>();
}

std::string ParseInitialPathUtf8(const json& payload) {
    if (!payload.contains("initial_path") || !payload["initial_path"].is_string()) {
        return {};
    }
    return payload["initial_path"].get<std::string>();
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

std::string ParseStringOrDefault(const json& payload, const char* key, std::string defaultValue = {}) {
    if (!payload.contains(key) || !payload[key].is_string()) {
        return defaultValue;
    }
    return payload[key].get<std::string>();
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

std::wstring NormalizeManifestPathForCompare(const std::wstring& path) {
    std::wstring normalized = path;
    for (wchar_t& ch : normalized) {
        if (ch == L'/') {
            ch = L'\\';
        }
        ch = static_cast<wchar_t>(std::towlower(ch));
    }
    return normalized;
}

bool IsSameManifestPath(const std::wstring& expected, const std::wstring& actual) {
    if (expected.empty() || actual.empty()) {
        return false;
    }

    std::error_code ecExpected;
    std::error_code ecActual;
    const std::wstring expectedCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(expected).lexically_normal().wstring());
    const std::wstring actualCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(actual).lexically_normal().wstring());
    if (!expectedCanonical.empty() && !actualCanonical.empty()) {
        return expectedCanonical == actualCanonical;
    }

    (void)ecExpected;
    (void)ecActual;
    return NormalizeManifestPathForCompare(expected) == NormalizeManifestPathForCompare(actual);
}

json BuildWasmResponse(AppController* controller, bool ok) {
    json body{{"ok", ok}};
    if (!controller) {
        return body;
    }

    const EffectConfig cfg = controller->GetConfigSnapshot();
    body["configured_enabled"] = cfg.wasm.enabled;
    body["fallback_to_builtin_click"] = cfg.wasm.fallbackToBuiltinClick;
    body["configured_manifest_path"] = cfg.wasm.manifestPath;
    body["configured_catalog_root_path"] = cfg.wasm.catalogRootPath;
    body["configured_output_buffer_bytes"] = cfg.wasm.outputBufferBytes;
    body["configured_max_commands"] = cfg.wasm.maxCommands;
    body["configured_max_execution_ms"] = cfg.wasm.maxEventExecutionMs;

    if (!controller->WasmHost()) {
        return body;
    }

    const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
    const wasm::ExecutionBudget runtimeBudget = controller->WasmHost()->GetExecutionBudget();
    body["enabled"] = diag.enabled;
    body["runtime_backend"] = diag.runtimeBackend;
    body["runtime_fallback_reason"] = diag.runtimeFallbackReason;
    body["plugin_loaded"] = diag.pluginLoaded;
    body["active_plugin_id"] = diag.activePluginId;
    body["active_manifest_path"] = Utf16ToUtf8(diag.activeManifestPath.c_str());
    body["runtime_output_buffer_bytes"] = runtimeBudget.outputBufferBytes;
    body["runtime_max_commands"] = runtimeBudget.maxCommands;
    body["runtime_max_execution_ms"] = runtimeBudget.maxEventExecutionMs;
    body["last_rendered_by_wasm"] = diag.lastRenderedByWasm;
    body["last_executed_text_commands"] = diag.lastExecutedTextCommands;
    body["last_executed_image_commands"] = diag.lastExecutedImageCommands;
    body["last_throttled_render_commands"] = diag.lastThrottledRenderCommands;
    body["last_throttled_by_capacity_render_commands"] = diag.lastThrottledByCapacityRenderCommands;
    body["last_throttled_by_interval_render_commands"] = diag.lastThrottledByIntervalRenderCommands;
    body["last_dropped_render_commands"] = diag.lastDroppedRenderCommands;
    body["last_render_error"] = diag.lastRenderError;
    body["last_error"] = diag.lastError;
    return body;
}

json BuildWasmActionResponse(AppController* controller, bool ok, const std::string& defaultError) {
    json body = BuildWasmResponse(controller, ok);
    if (ok) {
        return body;
    }

    std::string error = TrimAscii(defaultError);
    if (error.empty() && controller && controller->WasmHost()) {
        const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
        error = TrimAscii(diag.lastError);
        if (error.empty()) {
            error = TrimAscii(diag.lastRenderError);
        }
    }
    if (!error.empty()) {
        body["error"] = error;
    }
    return body;
}

std::vector<platform::ApplicationCatalogEntry> LoadAutomationAppCatalog(bool forceRefresh) {
    static std::mutex cacheMutex;
    static uint64_t cacheTickMs = 0;
    static std::vector<platform::ApplicationCatalogEntry> cacheEntries;

    constexpr uint64_t kCacheTtlMs = 30 * 1000;
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const uint64_t nowTickMs = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());

    std::lock_guard<std::mutex> lock(cacheMutex);
    if (!forceRefresh &&
        !cacheEntries.empty() &&
        (nowTickMs - cacheTickMs) < kCacheTtlMs) {
        return cacheEntries;
    }

    cacheEntries = platform::ScanApplicationCatalog();
    cacheTickMs = nowTickMs;
    return cacheEntries;
}

} // namespace

bool WebSettingsServer::HandleApiRoute(const HttpRequest& req, const std::string& path, HttpResponse& resp) {
    if (req.method == "GET" && path == "/api/schema") {
        SetJsonResponse(resp, controller_ ? BuildSettingsSchemaJson(controller_->GetConfigSnapshot()) : "{}");
        return true;
    }

    if (req.method == "GET" && path == "/api/state") {
        SetJsonResponse(resp, controller_ ? BuildSettingsStateJson(controller_->GetConfigSnapshot(), controller_) : "{}");
        return true;
    }

    if ((req.method == "POST" || req.method == "GET") && path == "/api/reload") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"reload_config\"}");
        }
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/stop") {
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        StopAsync();
        return true;
    }

    if (req.method == "POST" && path == "/api/reset") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"reset_config\"}");
        }
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/state") {
        SetJsonResponse(resp, ApplySettingsStateJson(controller_, req.body));
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/shortcut-capture/start") {
        if (!controller_) {
            SetJsonResponse(resp, json({{"ok", false}, {"error", "no controller"}}).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        uint64_t timeoutMs = 10000;
        if (payload.contains("timeout_ms") && payload["timeout_ms"].is_number_integer()) {
            const int64_t value = payload["timeout_ms"].get<int64_t>();
            if (value > 0) {
                timeoutMs = static_cast<uint64_t>(value);
            }
        }
        timeoutMs = std::clamp<uint64_t>(timeoutMs, 1000, 30000);

        const std::string sessionId = controller_->StartShortcutCaptureSession(timeoutMs);
        SetJsonResponse(resp, json({
            {"ok", !sessionId.empty()},
            {"session", sessionId}
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/shortcut-capture/poll") {
        if (!controller_) {
            SetJsonResponse(resp, json({{"ok", false}, {"error", "no controller"}}).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string sessionId = ParseSessionId(payload);
        const ShortcutCaptureSession::PollResult result = controller_->PollShortcutCaptureSession(sessionId);

        json body{
            {"ok", true},
            {"status", PollStateToText(result.state)}
        };
        if (!result.shortcut.empty()) {
            body["shortcut"] = result.shortcut;
        }
        SetJsonResponse(resp, body.dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/shortcut-capture/stop") {
        if (!controller_) {
            SetJsonResponse(resp, json({{"ok", false}, {"error", "no controller"}}).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string sessionId = ParseSessionId(payload);
        controller_->StopShortcutCaptureSession(sessionId);
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/active-process") {
        const std::string processBaseName = controller_
            ? controller_->CurrentForegroundProcessBaseName()
            : std::string{};
        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName}
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

        json selected = nullptr;
        if (match.binding != nullptr) {
            json normalizedScopes = json::array();
            for (const auto& scope : match.binding->appScopes) {
                normalizedScopes.push_back(automation_scope::NormalizeScopeToken(scope));
            }
            selected = json({
                {"index", static_cast<uint64_t>(match.bindingIndex)},
                {"trigger", match.binding->trigger},
                {"keys", match.binding->keys},
                {"app_scopes", match.binding->appScopes},
                {"app_scopes_normalized", normalizedScopes},
                {"chain_length", static_cast<uint64_t>(match.chainLength)},
                {"scope_specificity", match.scopeSpecificity},
            });
        }

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

    if (req.method == "POST" && path == "/api/automation/test-shortcut-from-mac-keycode") {
        if (!IsAutomationShortcutTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const int32_t rawMacKeyCode = ParseInt32OrDefault(payload, "mac_key_code", -1);
        const bool validMacKeyCode = rawMacKeyCode >= 0 && rawMacKeyCode <= static_cast<int32_t>(std::numeric_limits<uint16_t>::max());
        const uint32_t vkCode = validMacKeyCode
            ? macos_keymap::VirtualKeyFromMacKeyCode(static_cast<uint16_t>(rawMacKeyCode))
            : 0;

        KeyEvent ev{};
        ev.vkCode = vkCode;
        ev.ctrl = ParseBooleanOrDefault(payload, "ctrl", false);
        ev.shift = ParseBooleanOrDefault(payload, "shift", false);
        ev.alt = ParseBooleanOrDefault(payload, "alt", false);
        ev.win = ParseBooleanOrDefault(
            payload,
            "cmd",
            ParseBooleanOrDefault(payload, "win", false));
        ev.meta = ev.win;
        ev.systemKey = ev.alt || ev.meta;

        const std::string shortcut = shortcut_text::FormatShortcutText(ev);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"mac_key_code", rawMacKeyCode},
            {"vk_code", ev.vkCode},
            {"supported", validMacKeyCode && ev.vkCode != 0},
            {"shortcut", shortcut},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/test-inject-shortcut") {
        if (!IsAutomationInjectionTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller_) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string keys = ParseShortcutKeys(payload);
        const bool accepted = !keys.empty() && controller_->InjectShortcutForTest(keys);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"keys", keys},
            {"accepted", accepted},
            {"process", controller_->CurrentForegroundProcessBaseName()},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/input-indicator/test-mouse-labels") {
        if (!IsInputIndicatorTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller_) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        std::vector<std::string> labels;
        const bool supported = controller_->IndicatorOverlay().RunMouseLabelProbe(&labels);
        const bool matched = supported &&
                             labels.size() == 3 &&
                             labels[0] == "L" &&
                             labels[1] == "R" &&
                             labels[2] == "M";
        InputIndicatorDebugState debugState{};
        const bool debugStateOk = controller_->IndicatorOverlay().ReadDebugState(&debugState);

        SetJsonResponse(resp, json({
            {"ok", true},
            {"supported", supported},
            {"matched", matched},
            {"expected", json::array({"L", "R", "M"})},
            {"labels", labels},
            {"debug_state_available", debugStateOk},
            {"last_applied_label", debugStateOk ? debugState.lastAppliedLabel : std::string{}},
            {"apply_count", debugStateOk ? debugState.applyCount : 0},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/enable") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"wasm_enable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/test-dispatch-click") {
        if (!IsWasmTestDispatchApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller_ || !controller_->WasmHost()) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "wasm host unavailable"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::Click;
        invoke.x = ParseInt32OrDefault(payload, "x", 0);
        invoke.y = ParseInt32OrDefault(payload, "y", 0);
        invoke.button = ParseButtonOrDefault(payload, "button", 1);
        invoke.eventTickMs = controller_->CurrentTickMs();

        wasm::WasmEffectHost* host = controller_->WasmHost();
        const wasm::EventDispatchExecutionResult dispatchResult =
            wasm::InvokeEventAndRender(*host, invoke, controller_->Config());
        const wasm::HostDiagnostics& diag = host->Diagnostics();

        SetJsonResponse(resp, json({
            {"ok", true},
            {"route_active", dispatchResult.routeActive},
            {"invoke_ok", dispatchResult.invokeOk},
            {"rendered_any", dispatchResult.render.renderedAny},
            {"executed_text_commands", dispatchResult.render.executedTextCommands},
            {"executed_image_commands", dispatchResult.render.executedImageCommands},
            {"throttled_commands", dispatchResult.render.throttledCommands},
            {"throttled_by_capacity_commands", dispatchResult.render.throttledByCapacityCommands},
            {"throttled_by_interval_commands", dispatchResult.render.throttledByIntervalCommands},
            {"dropped_commands", dispatchResult.render.droppedCommands},
            {"render_error", dispatchResult.render.lastError},
            {"plugin_loaded", diag.pluginLoaded},
            {"runtime_backend", diag.runtimeBackend},
            {"last_error", diag.lastError},
            {"last_render_error", diag.lastRenderError},
            {"last_output_bytes", diag.lastOutputBytes},
            {"last_command_count", diag.lastCommandCount},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/catalog") {
        wasm::WasmPluginCatalog catalog;
        std::wstring configuredCatalogRoot;
        if (controller_) {
            configuredCatalogRoot = Utf8ToWString(controller_->GetConfigSnapshot().wasm.catalogRootPath);
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
        if (controller_) {
            configuredCatalogRoot = Utf8ToWString(controller_->GetConfigSnapshot().wasm.catalogRootPath);
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

    if (req.method == "POST" && path == "/api/wasm/policy") {
        bool ok = false;
        if (controller_) {
            const json payload = ParseObjectOrEmpty(req.body);
            json cmd;
            cmd["cmd"] = "wasm_set_policy";
            if (payload.contains("enabled") && payload["enabled"].is_boolean()) {
                cmd["enabled"] = payload["enabled"].get<bool>();
            }
            if (payload.contains("fallback_to_builtin_click") && payload["fallback_to_builtin_click"].is_boolean()) {
                cmd["fallback_to_builtin_click"] = payload["fallback_to_builtin_click"].get<bool>();
            }
            if (payload.contains("manifest_path") && payload["manifest_path"].is_string()) {
                cmd["manifest_path"] = payload["manifest_path"].get<std::string>();
            }
            if (payload.contains("catalog_root_path") && payload["catalog_root_path"].is_string()) {
                cmd["catalog_root_path"] = payload["catalog_root_path"].get<std::string>();
            }
            if (payload.contains("output_buffer_bytes") && payload["output_buffer_bytes"].is_number_integer()) {
                const int64_t raw = payload["output_buffer_bytes"].get<int64_t>();
                cmd["output_buffer_bytes"] = (raw <= 0)
                    ? 0u
                    : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
            }
            if (payload.contains("max_commands") && payload["max_commands"].is_number_integer()) {
                const int64_t raw = payload["max_commands"].get<int64_t>();
                cmd["max_commands"] = (raw <= 0)
                    ? 0u
                    : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
            }
            if (payload.contains("max_execution_ms") && payload["max_execution_ms"].is_number()) {
                cmd["max_execution_ms"] = payload["max_execution_ms"].get<double>();
            }
            controller_->HandleCommand(cmd.dump());
            ok = true;
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, ok).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/disable") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"wasm_disable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/reload") {
        bool ok = false;
        std::string error = "no controller";
        if (controller_ && controller_->WasmHost()) {
            error.clear();
            controller_->HandleCommand("{\"cmd\":\"wasm_reload\"}");
            ok = controller_->WasmHost()->Diagnostics().lastError.empty();
            if (!ok) {
                error = controller_->WasmHost()->Diagnostics().lastError;
            }
        } else if (controller_) {
            error = "wasm host unavailable";
        }
        SetJsonResponse(resp, BuildWasmActionResponse(controller_, ok, error).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/load-manifest") {
        bool ok = false;
        std::string error = "no controller";
        if (controller_ && controller_->WasmHost()) {
            error.clear();
            const json payload = ParseObjectOrEmpty(req.body);
            std::string manifestPathUtf8;
            if (payload.contains("manifest_path") && payload["manifest_path"].is_string()) {
                manifestPathUtf8 = payload["manifest_path"].get<std::string>();
            }
            if (!manifestPathUtf8.empty()) {
                json cmd;
                cmd["cmd"] = "wasm_load_manifest";
                cmd["manifest_path"] = manifestPathUtf8;
                controller_->HandleCommand(cmd.dump());
                const wasm::HostDiagnostics& diag = controller_->WasmHost()->Diagnostics();
                ok = diag.pluginLoaded &&
                    IsSameManifestPath(Utf8ToWString(manifestPathUtf8), diag.activeManifestPath);
                if (!ok) {
                    error = diag.lastError;
                    if (error.empty()) {
                        error = "manifest switch did not take effect";
                    }
                }
            } else {
                error = "manifest_path required";
            }
        } else if (controller_) {
            error = "wasm host unavailable";
        }
        SetJsonResponse(resp, BuildWasmActionResponse(controller_, ok, error).dump());
        return true;
    }

    return false;
}

bool WebSettingsServer::HandleStaticAssetRoute(const HttpRequest& req, HttpResponse& resp) {
    WebUiAsset asset;
    if (!assets_ || !assets_->TryGet(req.path, asset)) {
        return false;
    }

    resp.statusCode = 200;
    resp.contentType = asset.contentType;
    resp.body.assign(reinterpret_cast<const char*>(asset.bytes.data()), asset.bytes.size());
    return true;
}

void WebSettingsServer::HandleRequest(const HttpRequest& req, HttpResponse& resp) {
    Touch();

    try {
        const std::string path = StripQueryString(req.path);
        const bool isApi = (path.rfind("/api/", 0) == 0);
        if (isApi) {
            auto it = req.headers.find("x-mfcmouseeffect-token");
            const std::string token = (it == req.headers.end()) ? "" : TrimAscii(it->second);
            if (!IsTokenValid(token)) {
                SetPlainResponse(resp, 401, "unauthorized");
                return;
            }
        }

        if (HandleApiRoute(req, path, resp)) {
            return;
        }

        if (req.method == "GET" && path == "/favicon.ico") {
            resp.statusCode = 204;
            resp.contentType = "text/plain; charset=utf-8";
            resp.body.clear();
            return;
        }

        if (HandleStaticAssetRoute(req, resp)) {
            return;
        }

        SetPlainResponse(resp, 404, "not found");
    } catch (const std::exception& e) {
        const bool isApi = (StripQueryString(req.path).rfind("/api/", 0) == 0);
        resp.statusCode = 500;
        if (isApi) {
            resp.contentType = "application/json; charset=utf-8";
            resp.body = json({{"ok", false}, {"error", e.what()}}).dump();
            return;
        }
        resp.contentType = "text/plain; charset=utf-8";
        resp.body = e.what();
    }
}

} // namespace mousefx
