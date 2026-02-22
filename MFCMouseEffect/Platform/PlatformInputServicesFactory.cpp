#include "pch.h"

#include "Platform/PlatformInputServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32GlobalMouseHook.h"
#else
#include "MouseFx/Core/System/NullGlobalMouseHook.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IGlobalMouseHook> CreateGlobalMouseHook() {
#if defined(_WIN32)
    return std::make_unique<Win32GlobalMouseHook>();
#else
    return std::make_unique<NullGlobalMouseHook>();
#endif
}

} // namespace mousefx::platform
