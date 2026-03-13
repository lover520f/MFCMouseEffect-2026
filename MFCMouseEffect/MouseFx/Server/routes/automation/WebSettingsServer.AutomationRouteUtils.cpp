#include "pch.h"
#include "WebSettingsServer.AutomationRouteUtils.h"

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "MouseFx/Server/http/HttpServer.h"

using json = nlohmann::json;

namespace mousefx::websettings_automation_routes {

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
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

std::vector<platform::ApplicationCatalogEntry> LoadAutomationAppCatalog(bool forceRefresh) {
    static std::mutex cacheMutex;
    static uint64_t cacheTickMs = 0;
    static std::vector<platform::ApplicationCatalogEntry> cacheEntries;

    constexpr uint64_t kCacheTtlMs = 30 * 1000;
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const uint64_t nowTickMs =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());

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

} // namespace mousefx::websettings_automation_routes
