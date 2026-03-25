#include "pch.h"

#include "Platform/PlatformHoldRuntimeFactory.h"

#include "MouseFx/Effects/HoldRouteCatalog.h"

#if defined(_WIN32) && defined(MFX_ENABLE_WINDOWS_GPU_EFFECTS) && MFX_ENABLE_WINDOWS_GPU_EFFECTS
#include "Platform/windows/Effects/Win32HoldQuantumHaloGpuV2DirectRuntime.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IHoldRuntime> CreatePlatformHoldRuntime(const std::string& type) {
#if defined(_WIN32) && defined(MFX_ENABLE_WINDOWS_GPU_EFFECTS) && MFX_ENABLE_WINDOWS_GPU_EFFECTS
    if (hold_route::IsQuantumHaloGpuV2DirectType(type)) {
        return std::make_unique<Win32HoldQuantumHaloGpuV2DirectRuntime>();
    }
#endif
    (void)type;
    return nullptr;
}

} // namespace mousefx::platform
