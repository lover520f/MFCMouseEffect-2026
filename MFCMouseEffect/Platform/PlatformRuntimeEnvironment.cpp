#include "pch.h"

#include "Platform/PlatformRuntimeEnvironment.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32RuntimeEnvironment.h"
#endif

namespace mousefx::platform {

std::wstring GetExecutableDirectoryW() {
#if defined(_WIN32)
    return windows::GetExecutableDirectoryW();
#else
    return {};
#endif
}

std::wstring GetParentDirectoryW(const std::wstring& path) {
#if defined(_WIN32)
    return windows::GetParentDirectoryW(path);
#else
    (void)path;
    return {};
#endif
}

RuntimeProbeResult ProbeDawnRuntimeOnce() {
#if defined(_WIN32)
    return windows::ProbeDawnRuntimeOnce();
#else
    RuntimeProbeResult result{};
    result.available = false;
    result.reason = "runtime_probe_not_supported";
    return result;
#endif
}

} // namespace mousefx::platform
