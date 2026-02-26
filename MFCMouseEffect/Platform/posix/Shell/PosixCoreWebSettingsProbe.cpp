#include "pch.h"

#include "Platform/posix/Shell/PosixCoreWebSettingsProbe.h"
#include "Platform/posix/Shell/PosixKeyValueCaptureFile.h"

#include "MouseFx/Server/WebSettingsServer.h"

#include <cstdlib>
#include <string_view>
#include <vector>

namespace mousefx::platform {
namespace {

constexpr std::string_view kProbeFileEnv = "MFX_CORE_WEB_SETTINGS_PROBE_FILE";
constexpr std::string_view kLaunchProbeFileEnv = "MFX_CORE_WEB_SETTINGS_LAUNCH_PROBE_FILE";

std::string ReadProbeFilePathFromEnv(std::string_view envKey) {
    const char* filePath = std::getenv(envKey.data());
    if (filePath == nullptr || filePath[0] == '\0') {
        return {};
    }
    return filePath;
}

bool WriteProbeFile(
    const std::string& filePath,
    const std::vector<std::pair<std::string, std::string>>& lines) {
    return WritePosixKeyValueCaptureFile(filePath, lines);
}

} // namespace

std::string ReadCoreWebSettingsProbeFilePath() {
    return ReadProbeFilePathFromEnv(kProbeFileEnv);
}

bool WriteCoreWebSettingsProbeFile(const std::string& filePath, const WebSettingsServer& webSettings) {
    const std::string token = webSettings.Token();
    const std::string url = webSettings.Url();
    return WriteProbeFile(filePath, {
                                   {"url", url},
                                   {"token", token},
                                   {"port", std::to_string(webSettings.Port())},
                               });
}

std::string ReadCoreWebSettingsLaunchProbeFilePath() {
    return ReadProbeFilePathFromEnv(kLaunchProbeFileEnv);
}

bool WriteCoreWebSettingsLaunchProbeFile(
    const std::string& filePath,
    const std::string& settingsUrl,
    bool opened) {
    return WriteProbeFile(filePath, {
                                   {"url", settingsUrl},
                                   {"opened", opened ? "1" : "0"},
                               });
}

} // namespace mousefx::platform
