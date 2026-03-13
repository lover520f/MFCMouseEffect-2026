#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRuntime.Internal.h"

#include "MouseFx/Server/http/HttpServer.h"
#include "Platform/posix/Shell/ScaffoldSettingsRequestHandler.h"
#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.h"

#include <memory>
#include <string>

namespace mousefx::platform::scaffold::runtime_internal {

bool StartEmbeddedServer(
    SettingsRequestHandler& requestHandler,
    std::unique_ptr<HttpServer>& server,
    const ScaffoldSettingsRuntime::WarningSink& warningSink) {
    if (!requestHandler.Route().useEmbeddedServer) {
        return true;
    }

    if (!requestHandler.HasWebUiBaseDir() && warningSink) {
        warningSink("MFCMouseEffect", scaffold::BuildMissingWebUiMessage());
    }

    if (!server) {
        server = std::make_unique<HttpServer>();
    }
    if (server->IsRunning()) {
        return true;
    }

    const bool started = server->StartLoopbackOnPort(
        requestHandler.Route().port,
        [&requestHandler](const HttpRequest& req, HttpResponse& resp) {
            requestHandler.HandleRequest(req, resp);
        });
    if (!started && warningSink) {
        const std::string message =
            "Scaffold settings server failed to start (stage=" +
            std::to_string(server->LastStartErrorStage()) +
            ",code=" +
            std::to_string(server->LastStartErrorCode()) +
            ").";
        warningSink("MFCMouseEffect", message);
    }
    return started;
}

} // namespace mousefx::platform::scaffold::runtime_internal
