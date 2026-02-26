#include "Platform/posix/Shell/PosixSettingsLauncher.Internal.h"
#include "Platform/posix/Shell/PosixKeyValueCaptureFile.h"

#include <cstdlib>
#include <string_view>

namespace mousefx {
namespace {

constexpr std::string_view kLaunchCaptureFileEnv = "MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE";

} // namespace

bool IsLaunchInputValid(std::string_view url) {
    if (url.empty()) {
        return false;
    }

    for (unsigned char c : url) {
        if (c < 0x20 || c == 0x7f) {
            return false;
        }
    }

    return true;
}

std::string ReadLaunchCaptureFilePath() {
    const char* filePath = std::getenv(kLaunchCaptureFileEnv.data());
    if (filePath == nullptr || filePath[0] == '\0') {
        return {};
    }
    return filePath;
}

bool WriteLaunchCaptureFile(const std::string& filePath, const char* command, const std::string& url) {
    if (filePath.empty() || command == nullptr || command[0] == '\0') {
        return false;
    }
    return WritePosixKeyValueCaptureFile(filePath, {
                                                       {"command", command},
                                                       {"url", url},
                                                       {"captured", "1"},
                                                   });
}

} // namespace mousefx
