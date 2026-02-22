#include "pch.h"

#include "Platform/PlatformControlServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/Control/Win32DispatchMessageHost.h"
#else
#include "MouseFx/Core/Control/NullDispatchMessageHost.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IDispatchMessageHost> CreateDispatchMessageHost() {
#if defined(_WIN32)
    return std::make_unique<Win32DispatchMessageHost>();
#else
    return std::make_unique<NullDispatchMessageHost>();
#endif
}

} // namespace mousefx::platform
