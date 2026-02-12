#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct GpuFinalPresentCapability {
    bool dcompDllLoaded = false;
    bool dcompCreateDeviceProc = false;
    bool d3d11DllLoaded = false;
    bool d3d11CreateDeviceProc = false;
    bool dxgiDllLoaded = false;
    bool createDxgiFactoryProc = false;
    bool likelyAvailable = false;
    uint64_t probeTickMs = 0;
    std::string detail = "not_probed";
};

GpuFinalPresentCapability GetGpuFinalPresentCapability(bool refresh = false);

} // namespace mousefx::gpu
