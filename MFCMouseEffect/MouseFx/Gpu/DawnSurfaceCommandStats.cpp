#include "pch.h"

#include "DawnSurfaceCommandStats.h"

#include <sstream>

#include "OverlayGpuCommandStream.h"

namespace mousefx::gpu {

DawnSurfaceCommandStats BuildDawnSurfaceCommandStats(const OverlayGpuCommandStream* stream) {
    DawnSurfaceCommandStats out{};
    if (!stream) return out;
    const auto& cmds = stream->Commands();
    out.commandCount = static_cast<uint32_t>(cmds.size());
    for (const auto& cmd : cmds) {
        switch (cmd.type) {
        case OverlayGpuCommandType::TrailPolyline:
            ++out.trailCount;
            break;
        case OverlayGpuCommandType::RipplePulse:
            ++out.rippleCount;
            break;
        case OverlayGpuCommandType::ParticleSprites:
            ++out.particleCount;
            break;
        default:
            break;
        }
    }
    return out;
}

std::string BuildDawnSurfaceCommandStatsSuffix(const DawnSurfaceCommandStats& stats) {
    std::ostringstream oss;
    oss << "_cmd" << stats.commandCount
        << "_t" << stats.trailCount
        << "_r" << stats.rippleCount
        << "_p" << stats.particleCount;
    return oss.str();
}

} // namespace mousefx::gpu

