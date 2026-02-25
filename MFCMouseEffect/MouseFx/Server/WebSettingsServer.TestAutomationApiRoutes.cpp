#include "pch.h"
#include "WebSettingsServer.TestAutomationApiRoutes.h"

#include <string>

#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestAutomationInjectionApiRoutes.h"
#include "MouseFx/Server/WebSettingsServer.TestAutomationScopeApiRoutes.h"
#include "MouseFx/Server/WebSettingsServer.TestAutomationShortcutApiRoutes.h"

namespace mousefx {

bool HandleWebSettingsTestAutomationApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsTestAutomationScopeApiRoute(req, path, resp)) {
        return true;
    }

    if (HandleWebSettingsTestAutomationInjectionApiRoute(req, path, controller, resp)) {
        return true;
    }

    return HandleWebSettingsTestAutomationShortcutApiRoute(req, path, resp);
}

} // namespace mousefx
