#include "pch.h"

#include "Platform/PlatformOverlayServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/Overlay/Win32OverlayHostBackend.h"
#include "Platform/windows/Overlay/Win32InputIndicatorOverlay.h"
#else
#include "MouseFx/Core/Overlay/NullInputIndicatorOverlay.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IOverlayHostBackend> CreateOverlayHostBackend() {
#if defined(_WIN32)
    return std::make_unique<Win32OverlayHostBackend>();
#else
    return nullptr;
#endif
}

std::unique_ptr<IInputIndicatorOverlay> CreateInputIndicatorOverlay() {
#if defined(_WIN32)
    return std::make_unique<Win32InputIndicatorOverlay>();
#else
    return std::make_unique<NullInputIndicatorOverlay>();
#endif
}

} // namespace mousefx::platform
