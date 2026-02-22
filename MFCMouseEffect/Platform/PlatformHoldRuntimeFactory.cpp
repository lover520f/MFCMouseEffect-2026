#include "pch.h"

#include "Platform/PlatformHoldRuntimeFactory.h"

#include "MouseFx/Effects/HoldRouteCatalog.h"

#if defined(_WIN32)
#include "Platform/windows/Effects/Win32HoldQuantumHaloGpuV2DirectRuntime.h"
#endif

namespace mousefx::platform {

std::unique_ptr<IHoldRuntime> CreatePlatformHoldRuntime(const std::string& type) {
#if defined(_WIN32)
    if (hold_route::IsQuantumHaloGpuV2DirectType(type)) {
        return std::make_unique<Win32HoldQuantumHaloGpuV2DirectRuntime>();
    }
#endif
    (void)type;
    return nullptr;
}

} // namespace mousefx::platform
