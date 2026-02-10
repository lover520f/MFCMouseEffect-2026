#pragma once

#include <cstdint>

#include "MouseFx/Gpu/OverlayGpuCommandStream.h"

namespace mousefx::gpu {

struct RippleGeometryPrepResult {
    uint32_t batches = 0;
    uint32_t pulses = 0;
    uint32_t triangles = 0;
    uint32_t uploadBytes = 0;
};

inline RippleGeometryPrepResult PreprocessRippleGeometry(
    const OverlayGpuCommandStream& stream) {
    RippleGeometryPrepResult out{};
    constexpr uint32_t kBytesPerRipplePulse = 120; // approximated as a screen-space quad (6 verts)
    for (const auto& cmd : stream.Commands()) {
        if (cmd.type != OverlayGpuCommandType::RipplePulse || cmd.vertices.empty()) continue;
        ++out.batches;
        out.pulses += (uint32_t)cmd.vertices.size();
        out.triangles += (uint32_t)cmd.vertices.size() * 2u;
        const uint64_t bytes = (uint64_t)out.uploadBytes +
                               (uint64_t)cmd.vertices.size() * (uint64_t)kBytesPerRipplePulse;
        out.uploadBytes = (bytes > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)bytes;
    }
    return out;
}

} // namespace mousefx::gpu

