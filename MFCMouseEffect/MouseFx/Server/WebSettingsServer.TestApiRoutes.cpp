#include "pch.h"
#include "WebSettingsServer.TestApiRoutes.h"

#include "MouseFx/Server/WebSettingsServer.TestAutomationApiRoutes.h"
#include "MouseFx/Server/WebSettingsServer.TestEffectsApiRoutes.h"
#include "MouseFx/Server/WebSettingsServer.TestWasmInputApiRoutes.h"

namespace mousefx {

bool HandleWebSettingsTestApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsTestAutomationApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsTestEffectsApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsTestWasmInputApiRoute(req, path, controller, resp)) {
        return true;
    }

    return false;
}

} // namespace mousefx
