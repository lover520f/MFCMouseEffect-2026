#pragma once

#include <string>

namespace mousefx {
class WebSettingsServer;
}

namespace mousefx::platform {

std::string ReadCoreWebSettingsProbeFilePath();
bool WriteCoreWebSettingsProbeFile(const std::string& filePath, const WebSettingsServer& webSettings);
std::string ReadCoreWebSettingsLaunchProbeFilePath();
bool WriteCoreWebSettingsLaunchProbeFile(
    const std::string& filePath,
    const std::string& settingsUrl,
    bool opened);
std::string ReadCoreWebSettingsProbeDiagnosticsFilePath();
bool WriteCoreWebSettingsProbeDiagnosticsFile(
    const std::string& filePath,
    const std::string& status,
    const std::string& reason);

} // namespace mousefx::platform
