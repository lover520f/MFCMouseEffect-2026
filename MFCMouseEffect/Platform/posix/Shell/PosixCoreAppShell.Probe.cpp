#include "pch.h"

#include "Platform/posix/Shell/PosixCoreAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include "Platform/posix/Shell/PosixCoreWebSettingsProbe.h"
#include "MouseFx/Server/WebSettingsServer.h"

#include <string>

namespace mousefx::platform {

bool PosixCoreAppShell::SetupRegressionWebSettingsProbe() {
    const std::string probeFilePath = ReadCoreWebSettingsProbeFilePath();
    const std::string launchProbeFilePath = ReadCoreWebSettingsLaunchProbeFilePath();
    if (!appController_) {
        return true;
    }
    if (probeFilePath.empty() && launchProbeFilePath.empty()) {
        return true;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(appController_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            return false;
        }
    }

    bool ok = true;
    if (!probeFilePath.empty()) {
        ok = WriteCoreWebSettingsProbeFile(probeFilePath, *webSettings_) && ok;
    }

    if (!launchProbeFilePath.empty()) {
        const std::string settingsUrl = webSettings_->Url();
        const bool opened = services_.settingsLauncher && services_.settingsLauncher->OpenUrlUtf8(settingsUrl);
        ok = WriteCoreWebSettingsLaunchProbeFile(launchProbeFilePath, settingsUrl, opened) && ok;
        ok = opened && ok;
    }

    return ok;
}

} // namespace mousefx::platform

#endif
