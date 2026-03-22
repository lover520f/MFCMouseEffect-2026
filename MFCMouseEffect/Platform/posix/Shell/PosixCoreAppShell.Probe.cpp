#include "pch.h"

#include "Platform/posix/Shell/PosixCoreAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include "MouseFx/Server/core/WebSettingsServer.h"
#include "MouseFx/Core/Shell/WebSettingsLaunchCoordinator.h"
#include "Platform/posix/Shell/PosixCoreWebSettingsProbe.h"

#include <sstream>
#include <string>

namespace mousefx::platform {

bool PosixCoreAppShell::SetupRegressionWebSettingsProbe() {
    const std::string probeFilePath = ReadCoreWebSettingsProbeFilePath();
    const std::string launchProbeFilePath = ReadCoreWebSettingsLaunchProbeFilePath();
    const std::string diagnosticsFilePath = ReadCoreWebSettingsProbeDiagnosticsFilePath();
    const auto writeDiagnostics = [&](const char* status, const char* reason) {
        if (diagnosticsFilePath.empty()) {
            return true;
        }
        return WriteCoreWebSettingsProbeDiagnosticsFile(diagnosticsFilePath, status, reason);
    };
    if (!appController_) {
        writeDiagnostics("skipped", "app_controller_missing");
        return true;
    }
    if (probeFilePath.empty() && launchProbeFilePath.empty()) {
        writeDiagnostics("skipped", "probe_env_not_set");
        return true;
    }

    if (!webSettingsCoordinator_) {
        webSettingsCoordinator_ = std::make_unique<WebSettingsLaunchCoordinator>(appController_.get());
    }
    webSettingsCoordinator_->ResetController(appController_.get());
    const mousefx::WebSettingsLaunchResult launch = webSettingsCoordinator_->EnsureStarted();
    if (!launch.ok) {
        std::ostringstream reason;
        reason << launch.errorCode
               << "(stage=" << launch.startErrorStage
               << ",code=" << launch.startErrorCode << ")";
        const std::string reasonText = reason.str();
        writeDiagnostics("error", reasonText.c_str());
        return false;
    }

    const WebSettingsServer* server = webSettingsCoordinator_->Server();
    if (!server) {
        writeDiagnostics("error", "websettings_server_missing_after_start");
        return false;
    }

    bool ok = true;
    if (!probeFilePath.empty()) {
        const bool probeWriteOk = WriteCoreWebSettingsProbeFile(probeFilePath, *server);
        ok = probeWriteOk && ok;
        if (!probeWriteOk) {
            writeDiagnostics("error", "probe_file_write_failed");
        }
    }

    if (!launchProbeFilePath.empty()) {
        const std::string settingsUrl = server->Url();
        const bool opened = services_.settingsLauncher && services_.settingsLauncher->OpenUrlUtf8(settingsUrl);
        const bool launchProbeWriteOk =
            WriteCoreWebSettingsLaunchProbeFile(launchProbeFilePath, settingsUrl, opened);
        ok = launchProbeWriteOk && ok;
        if (!launchProbeWriteOk) {
            writeDiagnostics("error", "launch_probe_file_write_failed");
        }
    }

    if (ok) {
        writeDiagnostics("ok", "probe_files_written");
    }
    return ok;
}

} // namespace mousefx::platform

#endif
