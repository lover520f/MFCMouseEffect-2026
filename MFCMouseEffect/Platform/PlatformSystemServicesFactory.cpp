#include "pch.h"

#include "Platform/PlatformSystemServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32ForegroundProcessService.h"
#include "Platform/windows/System/Win32MonotonicClockService.h"
#else
#include "MouseFx/Core/System/NullForegroundProcessService.h"
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

std::unique_ptr<IForegroundProcessService> CreateForegroundProcessService() {
#if defined(_WIN32)
    return std::make_unique<Win32ForegroundProcessService>();
#else
    return std::make_unique<NullForegroundProcessService>();
#endif
}

} // namespace mousefx::platform
