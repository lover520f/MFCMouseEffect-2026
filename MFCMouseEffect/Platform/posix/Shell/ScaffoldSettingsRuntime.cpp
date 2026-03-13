#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRuntime.h"
#include "Platform/posix/Shell/ScaffoldSettingsRuntime.Internal.h"

#include "MouseFx/Server/http/HttpServer.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.h"
#include "Platform/posix/Shell/ScaffoldSettingsRequestHandler.h"
#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.h"

#include <string>

namespace mousefx::platform {

struct ScaffoldSettingsRuntime::Impl {
    scaffold::SettingsRequestHandler requestHandler{
        scaffold::BuildSettingsRoute(),
        scaffold::BuildWebUiBaseDirs(),
    };
    std::unique_ptr<HttpServer> server{};

    void SetRuntimeMode(bool tray, bool background) {
        requestHandler.SetRuntimeMode(tray, background);
    }

    bool Start(const ScaffoldSettingsRuntime::WarningSink& warningSink) {
        return scaffold::runtime_internal::StartEmbeddedServer(requestHandler, server, warningSink);
    }

    void Stop() {
        if (server) {
            server->Stop();
        }
    }
};

ScaffoldSettingsRuntime::ScaffoldSettingsRuntime()
    : impl_(std::make_unique<Impl>()) {
}

ScaffoldSettingsRuntime::~ScaffoldSettingsRuntime() = default;

void ScaffoldSettingsRuntime::SetRuntimeMode(bool trayAvailable, bool backgroundMode) {
    if (!impl_) {
        return;
    }
    impl_->SetRuntimeMode(trayAvailable, backgroundMode);
}

bool ScaffoldSettingsRuntime::Start(const WarningSink& warningSink) {
    if (!impl_) {
        return false;
    }
    return impl_->Start(warningSink);
}

void ScaffoldSettingsRuntime::Stop() {
    if (!impl_) {
        return;
    }
    impl_->Stop();
}

const std::string& ScaffoldSettingsRuntime::SettingsUrl() const {
    static const std::string kEmpty;
    if (!impl_) {
        return kEmpty;
    }
    return impl_->requestHandler.Route().url;
}

} // namespace mousefx::platform
