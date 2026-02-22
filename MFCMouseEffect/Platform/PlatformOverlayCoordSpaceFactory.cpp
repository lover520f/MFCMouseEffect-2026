#include "pch.h"

#include "Platform/PlatformOverlayCoordSpaceFactory.h"

#if defined(_WIN32)
#include "Platform/windows/Overlay/Win32OverlayCoordSpaceService.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IOverlayCoordSpaceService> CreateOverlayCoordSpaceService() {
#if defined(_WIN32)
    return std::make_unique<Win32OverlayCoordSpaceService>();
#else
    return nullptr;
#endif
}

} // namespace mousefx::platform
