#include "pch.h"
#include "WebSettingsServer.TestApiRoutes.h"

#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationApiRoutes.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestEffectsApiRoutes.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestWasmInputApiRoutes.h"

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
