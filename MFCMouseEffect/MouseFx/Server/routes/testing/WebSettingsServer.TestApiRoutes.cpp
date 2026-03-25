#include "pch.h"
#include "WebSettingsServer.TestApiRoutes.h"

#if !defined(MFX_SHIPPING_BUILD)
#include "MouseFx/Server/routes/automation/WebSettingsServer.TestAutomationApiRoutes.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestEffectsApiRoutes.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestMouseCompanionApiRoutes.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestWasmInputApiRoutes.h"
#endif

namespace mousefx {

bool HandleWebSettingsTestApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
#if defined(MFX_SHIPPING_BUILD)
    (void)req;
    (void)path;
    (void)controller;
    (void)resp;
    return false;
#else
    if (HandleWebSettingsTestAutomationApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsTestEffectsApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsTestMouseCompanionApiRoute(req, path, controller, resp)) {
        return true;
    }

    if (HandleWebSettingsTestWasmInputApiRoute(req, path, controller, resp)) {
        return true;
    }

    return false;
#endif
}

} // namespace mousefx
