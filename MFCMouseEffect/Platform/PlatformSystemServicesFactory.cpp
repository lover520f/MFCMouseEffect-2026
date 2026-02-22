#include "pch.h"

#include "Platform/PlatformSystemServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32MonotonicClockService.h"
#else
#include "MouseFx/Core/System/StdMonotonicClockService.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IMonotonicClockService> CreateMonotonicClockService() {
#if defined(_WIN32)
    return std::make_unique<Win32MonotonicClockService>();
#else
    return std::make_unique<StdMonotonicClockService>();
#endif
}

} // namespace mousefx::platform
