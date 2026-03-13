#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsApi.h"

#include "MouseFx/Server/http/HttpServer.h"
#include "Platform/PlatformTarget.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.h"

namespace mousefx::platform::scaffold {
namespace {

json BuildRouteJson(const SettingsRoute& route) {
    const std::string tokenQuery = BuildTokenQuerySuffix(route.token);
    return json{
        {"schema", std::string("/api/schema") + tokenQuery},
        {"state", std::string("/api/state") + tokenQuery},
        {"health", std::string("/api/health") + tokenQuery},
    };
}

const char* PlatformName() {
#if MFX_PLATFORM_MACOS
    return "macos";
#elif MFX_PLATFORM_LINUX
    return "linux";
#else
    return "unknown";
#endif
}

} // namespace

void SetJsonResponse(HttpResponse& resp, int statusCode, const json& payload) {
    resp.statusCode = statusCode;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = payload.dump();
}

void SetPlainResponse(HttpResponse& resp, int statusCode, const std::string& body) {
    resp.statusCode = statusCode;
    resp.contentType = "text/plain; charset=utf-8";
    resp.body = body;
}

json BuildHealthJson(const SettingsRoute& route) {
    return json{
        {"ok", true},
        {"status", "ready"},
        {"service", "scaffold-settings"},
        {"platform", PlatformName()},
        {"entry_url", route.url},
        {"routes", BuildRouteJson(route)},
    };
}

json BuildSchemaJson(const SettingsRoute& route) {
    return json{
        {"ok", true},
        {"mode", "scaffold"},
        {"api_version", 1},
        {"token_required", !route.token.empty()},
        {"routes", BuildRouteJson(route)},
        {"capabilities", {
            {"read_state", true},
            {"write_state", true},
            {"apply_settings", false},
        }},
        {"fields", json::array({
            json{{"key", "ui_language"}, {"type", "select"}, {"default", "zh-CN"},
                 {"options", json::array({"zh-CN", "en-US"})}},
            json{{"key", "theme"}, {"type", "select"}, {"default", "scaffold"},
                 {"options", json::array({"scaffold"})}},
            json{{"key", "hold_follow_mode"}, {"type", "select"}, {"default", "precise"},
                 {"options", json::array({"precise", "smooth", "efficient"})}},
            json{{"key", "tray_mode"}, {"type", "readonly"}},
        })},
    };
}

json BuildStateJson(
    const SettingsRoute& route,
    const RuntimeState& state,
    bool trayAvailable,
    bool backgroundMode,
    uint64_t revision) {
    return json{
        {"ok", true},
        {"mode", "scaffold"},
        {"platform", PlatformName()},
        {"entry_url", route.url},
        {"entry_path", route.path},
        {"token_required", !route.token.empty()},
        {"tray_available", trayAvailable},
        {"runtime_mode", backgroundMode ? "background" : "tray"},
        {"state_revision", revision},
        {"state", {
            {"ui_language", state.uiLanguage},
            {"theme", state.theme},
            {"hold_follow_mode", state.holdFollowMode},
        }},
        {"write_enabled", true},
    };
}

json BuildErrorJson(const std::string& code, const std::string& message) {
    return json{{"ok", false}, {"error", code}, {"message", message}};
}

} // namespace mousefx::platform::scaffold
