#include "pch.h"
#include "WebSettingsServer.TestEffectsApiRoutes.h"

#include "MouseFx/Server/routes/testing/WebSettingsServer.TestEffectsOverlayApiRoute.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestEffectsProfileApiRoute.h"

namespace mousefx {

bool HandleWebSettingsTestEffectsApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (HandleWebSettingsTestEffectsProfileApiRoute(req, path, controller, resp)) {
        return true;
    }
    if (HandleWebSettingsTestEffectsOverlayApiRoute(req, path, controller, resp)) {
        return true;
    }
    return false;
}

} // namespace mousefx
