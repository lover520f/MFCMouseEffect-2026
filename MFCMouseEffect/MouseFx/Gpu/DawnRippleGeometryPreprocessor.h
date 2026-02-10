#pragma once

#include <cstdint>
#include <vector>

#include "MouseFx/Gpu/OverlayGpuCommandStream.h"

namespace mousefx::gpu {

struct RippleGeometryPrepResult {
    uint32_t batches = 0;
    uint32_t pulses = 0;
    uint32_t triangles = 0;
    uint32_t uploadBytes = 0;
    uint32_t bakedQuads = 0;
    uint32_t bakedVertices = 0;
};

inline RippleGeometryPrepResult PreprocessRippleGeometry(
    const OverlayGpuCommandStream& stream) {
    RippleGeometryPrepResult out{};
    // 6 vertices * (x, y, t, color-as-float) * 4 bytes
    constexpr uint32_t kBytesPerRipplePulse = 96;
    std::vector<float> bakedScratch{};
    bakedScratch.reserve(6 * 4 * 32);
    for (const auto& cmd : stream.Commands()) {
        if (cmd.type != OverlayGpuCommandType::RipplePulse || cmd.vertices.empty()) continue;
        ++out.batches;
        bakedScratch.clear();
        bakedScratch.reserve(cmd.vertices.size() * 6u * 4u);
        for (const auto& v : cmd.vertices) {
            const float cx = v.x;
            const float cy = v.y;
            const float radius = (v.size > 1.0f) ? v.size : 32.0f;
            const float t = v.extra;
            const float color = (float)v.colorArgb;

            const float l = cx - radius;
            const float r = cx + radius;
            const float top = cy - radius;
            const float btm = cy + radius;

            // Two triangles (CCW): LT-RT-LB, LB-RT-RB
            bakedScratch.push_back(l);   bakedScratch.push_back(top); bakedScratch.push_back(t); bakedScratch.push_back(color);
            bakedScratch.push_back(r);   bakedScratch.push_back(top); bakedScratch.push_back(t); bakedScratch.push_back(color);
            bakedScratch.push_back(l);   bakedScratch.push_back(btm); bakedScratch.push_back(t); bakedScratch.push_back(color);
            bakedScratch.push_back(l);   bakedScratch.push_back(btm); bakedScratch.push_back(t); bakedScratch.push_back(color);
            bakedScratch.push_back(r);   bakedScratch.push_back(top); bakedScratch.push_back(t); bakedScratch.push_back(color);
            bakedScratch.push_back(r);   bakedScratch.push_back(btm); bakedScratch.push_back(t); bakedScratch.push_back(color);

            ++out.pulses;
            ++out.bakedQuads;
            out.bakedVertices += 6u;
            out.triangles += 2u;
        }
        const uint64_t bytes = (uint64_t)out.uploadBytes + (uint64_t)bakedScratch.size() * sizeof(float);
        out.uploadBytes = (bytes > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)bytes;
    }
    return out;
}

} // namespace mousefx::gpu
