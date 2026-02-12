#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

class OverlayGpuCommandStream;

struct DawnSurfaceCommandStats {
    uint32_t commandCount = 0;
    uint32_t trailCount = 0;
    uint32_t rippleCount = 0;
    uint32_t particleCount = 0;
};

DawnSurfaceCommandStats BuildDawnSurfaceCommandStats(const OverlayGpuCommandStream* stream);
std::string BuildDawnSurfaceCommandStatsSuffix(const DawnSurfaceCommandStats& stats);

} // namespace mousefx::gpu

