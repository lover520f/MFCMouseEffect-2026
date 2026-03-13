#pragma once

#include "Platform/posix/Shell/ScaffoldSettingsRuntime.h"

#include <memory>

namespace mousefx {
class HttpServer;
}

namespace mousefx::platform::scaffold {
class SettingsRequestHandler;
}

namespace mousefx::platform::scaffold::runtime_internal {

bool StartEmbeddedServer(
    SettingsRequestHandler& requestHandler,
    std::unique_ptr<HttpServer>& server,
    const ScaffoldSettingsRuntime::WarningSink& warningSink);

} // namespace mousefx::platform::scaffold::runtime_internal
