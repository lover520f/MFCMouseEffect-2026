#include "pch.h"

#include "Platform/PlatformLaunchAtStartup.h"

#include "Platform/PlatformTarget.h"

#if MFX_PLATFORM_MACOS
#include "Platform/macos/System/MacosLaunchAtStartupSwiftBridge.h"
#endif

#include <array>
#include <string>

namespace mousefx::platform {

bool IsLaunchAtStartupSupported() {
#if MFX_PLATFORM_MACOS
    return true;
#else
    return false;
#endif
}

bool ConfigureLaunchAtStartup(bool enabled, std::string* outError) {
    if (outError) {
        outError->clear();
    }

#if MFX_PLATFORM_MACOS
    constexpr int32_t kErrorBufferCapacity = 2048;
    std::array<char, kErrorBufferCapacity> errorBuffer{};
    const int32_t outcome =
        mfx_macos_set_launch_at_startup_v1(enabled ? 1 : 0, errorBuffer.data(), kErrorBufferCapacity);
    if (outcome > 0) {
        return true;
    }
    if (outError) {
        *outError = errorBuffer.data();
        if (outError->empty()) {
            *outError = "launch_at_startup_apply_failed";
        }
    }
    return false;
#else
    if (outError) {
        *outError = "launch_at_startup_not_supported";
    }
    return false;
#endif
}

} // namespace mousefx::platform

