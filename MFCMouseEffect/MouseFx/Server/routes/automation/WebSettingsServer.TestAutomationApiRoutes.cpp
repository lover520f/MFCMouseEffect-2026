#include "pch.h"
#include "WebSettingsServer.TestAutomationApiRoutes.h"

#include <string>

#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationInjectionApiRoutes.h"
#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationScopeApiRoutes.h"
#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationShortcutApiRoutes.h"

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
