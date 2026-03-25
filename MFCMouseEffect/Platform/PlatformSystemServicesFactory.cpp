#include "pch.h"

#include "Platform/PlatformSystemServicesFactory.h"
#include "Platform/PlatformTarget.h"
#include "MouseFx/Core/System/NullForegroundSuppressionService.h"

#if MFX_PLATFORM_WINDOWS
#include "Platform/windows/System/Win32ForegroundProcessService.h"
#include "Platform/windows/System/Win32MonotonicClockService.h"
#elif MFX_PLATFORM_MACOS
#include "MouseFx/Core/System/StdMonotonicClockService.h"
#include "Platform/macos/System/MacosForegroundProcessService.h"
#else
#include "MouseFx/Core/System/NullForegroundProcessService.h"
#include "MouseFx/Core/System/StdMonotonicClockService.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IMonotonicClockService> CreateMonotonicClockService() {
#if MFX_PLATFORM_WINDOWS
    return std::make_unique<Win32MonotonicClockService>();
#else
    return std::make_unique<StdMonotonicClockService>();
#endif
}

std::unique_ptr<IForegroundProcessService> CreateForegroundProcessService() {
#if MFX_PLATFORM_WINDOWS
    return std::make_unique<Win32ForegroundProcessService>();
#elif MFX_PLATFORM_MACOS
    return std::make_unique<MacosForegroundProcessService>();
#else
    return std::make_unique<NullForegroundProcessService>();
#endif
}

std::unique_ptr<IForegroundSuppressionService> CreateForegroundSuppressionService() {
    return std::make_unique<NullForegroundSuppressionService>();
}

} // namespace mousefx::platform
