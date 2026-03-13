#include "pch.h"

#include "Platform/posix/Shell/PosixCoreAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include "Platform/posix/Shell/PosixCoreWebSettingsProbe.h"
#include "MouseFx/Server/core/WebSettingsServer.h"

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

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(appController_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            std::ostringstream reason;
            reason << "websettings_start_failed"
                   << "(stage=" << webSettings_->LastStartErrorStage()
                   << ",code=" << webSettings_->LastStartErrorCode() << ")";
            const std::string reasonText = reason.str();
            writeDiagnostics("error", reasonText.c_str());
            return false;
        }
    }

    bool ok = true;
    if (!probeFilePath.empty()) {
        const bool probeWriteOk = WriteCoreWebSettingsProbeFile(probeFilePath, *webSettings_);
        ok = probeWriteOk && ok;
        if (!probeWriteOk) {
            writeDiagnostics("error", "probe_file_write_failed");
        }
    }

    if (!launchProbeFilePath.empty()) {
        const std::string settingsUrl = webSettings_->Url();
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
