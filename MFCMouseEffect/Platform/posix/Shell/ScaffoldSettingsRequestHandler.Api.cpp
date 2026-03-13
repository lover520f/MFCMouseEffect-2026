#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRequestHandler.h"
#include "MouseFx/Server/http/HttpServer.h"

namespace mousefx::platform::scaffold {

bool SettingsRequestHandler::TryHandleApiRequest(
    const HttpRequest& req,
    const std::string& pathOnly,
    HttpResponse& resp) {
    if (req.method == "GET" && pathOnly == "/api/health") {
        SetJsonResponse(resp, 200, BuildHealthJson(route_));
        return true;
    }
    if (req.method == "GET" && pathOnly == "/api/schema") {
        SetJsonResponse(resp, 200, BuildSchemaJson(route_));
        return true;
    }
    if (req.method == "GET" && pathOnly == "/api/state") {
        RuntimeState stateSnapshot;
        uint64_t revision = 0;
        bool tray = false;
        bool background = false;
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            stateSnapshot = runtimeState_;
            revision = runtimeStateRevision_;
            tray = trayAvailable_;
            background = backgroundMode_;
        }
        SetJsonResponse(resp, 200, BuildStateJson(route_, stateSnapshot, tray, background, revision));
        return true;
    }
    if (req.method == "POST" && pathOnly == "/api/state") {
        RuntimeState nextState;
        uint64_t revision = 0;
        bool tray = false;
        bool background = false;
        json error;
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            if (!ParseStatePatch(req.body, runtimeState_, &nextState, &error)) {
                SetJsonResponse(resp, 400, error);
                return true;
            }
            runtimeState_ = nextState;
            ++runtimeStateRevision_;
            revision = runtimeStateRevision_;
            tray = trayAvailable_;
            background = backgroundMode_;
        }
        SetJsonResponse(resp, 200, BuildStateJson(route_, nextState, tray, background, revision));
        return true;
    }

    return false;
}

} // namespace mousefx::platform::scaffold
