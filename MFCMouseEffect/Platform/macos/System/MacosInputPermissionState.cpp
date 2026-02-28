#include "pch.h"

#include "Platform/macos/System/MacosInputPermissionState.h"
#include "Platform/macos/System/MacosInputPermissionState.Internal.h"
#include "Platform/macos/System/MacosInputPermissionSwiftBridge.h"

#include <cstdlib>
#include <string_view>

namespace mousefx::macos_input_permission {

namespace {

constexpr std::string_view kPermissionSimulationFileEnv =
    "MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE";

} // namespace

std::string ReadPermissionSimulationFilePath() {
    const char* raw = std::getenv(kPermissionSimulationFileEnv.data());
    if (raw == nullptr || raw[0] == '\0') {
        return {};
    }
    return std::string(raw);
}

bool IsRuntimeInputTrusted(const std::string& simulationFilePath) {
    if (!simulationFilePath.empty()) {
        return permission_detail::ReadPermissionSimulationTrusted(simulationFilePath, true);
    }
#if defined(__APPLE__)
    return mfx_macos_is_process_trusted_v1() > 0;
#else
    return false;
#endif
}

} // namespace mousefx::macos_input_permission
