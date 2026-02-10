#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <future>
#include <thread>
#include <vector>

#include "MouseFx/Gpu/DawnRippleGeometryPreprocessor.h"
#include "MouseFx/Gpu/OverlayGpuCommandStream.h"

namespace mousefx::gpu {

struct TrailGeometryPrepResult {
    uint32_t batches = 0;
    uint32_t vertices = 0;
    uint32_t segments = 0;
    uint32_t triangles = 0;
    uint32_t uploadBytes = 0;
    uint32_t particleBatches = 0;
    uint32_t particleSprites = 0;
    uint32_t particleUploadBytes = 0;
    uint32_t rippleBatches = 0;
    uint32_t ripplePulses = 0;
    uint32_t rippleTriangles = 0;
    uint32_t rippleUploadBytes = 0;
    uint32_t rippleBakedQuads = 0;
    uint32_t rippleBakedVertices = 0;
    uint32_t workers = 1;
    bool usedParallel = false;
};

namespace detail {

inline TrailGeometryPrepResult MergeTrailPrep(
    const TrailGeometryPrepResult& a,
    const TrailGeometryPrepResult& b) {
    TrailGeometryPrepResult out{};
    out.batches = a.batches + b.batches;
    out.vertices = a.vertices + b.vertices;
    out.segments = a.segments + b.segments;
    out.triangles = a.triangles + b.triangles;
    const uint64_t upload = (uint64_t)a.uploadBytes + (uint64_t)b.uploadBytes;
    out.uploadBytes = (upload > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)upload;
    out.particleBatches = a.particleBatches + b.particleBatches;
    out.particleSprites = a.particleSprites + b.particleSprites;
    const uint64_t particleUpload = (uint64_t)a.particleUploadBytes + (uint64_t)b.particleUploadBytes;
    out.particleUploadBytes = (particleUpload > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)particleUpload;
    out.rippleBatches = a.rippleBatches + b.rippleBatches;
    out.ripplePulses = a.ripplePulses + b.ripplePulses;
    out.rippleTriangles = a.rippleTriangles + b.rippleTriangles;
    const uint64_t rippleUpload = (uint64_t)a.rippleUploadBytes + (uint64_t)b.rippleUploadBytes;
    out.rippleUploadBytes = (rippleUpload > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)rippleUpload;
    out.rippleBakedQuads = a.rippleBakedQuads + b.rippleBakedQuads;
    out.rippleBakedVertices = a.rippleBakedVertices + b.rippleBakedVertices;
    out.workers = (a.workers > b.workers) ? a.workers : b.workers;
    out.usedParallel = a.usedParallel || b.usedParallel;
    return out;
}

inline TrailGeometryPrepResult BuildTrailGeometryRange(
    const std::vector<const OverlayGpuCommand*>& trails,
    size_t begin,
    size_t end) {
    TrailGeometryPrepResult result{};
    constexpr size_t kMaxTrailVerticesPerBatch = 1024;
    constexpr float kBaseHalfWidth = 2.2f;
    std::vector<float> uploadScratch{};

    for (size_t t = begin; t < end; ++t) {
        const OverlayGpuCommand* cmd = trails[t];
        if (!cmd || cmd->vertices.size() < 2) continue;

        size_t offset = 0;
        while (offset + 1 < cmd->vertices.size()) {
            const size_t remain = cmd->vertices.size() - offset;
            const size_t take = (remain > kMaxTrailVerticesPerBatch) ? kMaxTrailVerticesPerBatch : remain;
            if (take < 2) break;

            ++result.batches;
            result.vertices += (uint32_t)take;
            result.segments += (uint32_t)(take - 1);

            const size_t localSegments = take - 1;
            const size_t outVertexCount = localSegments * 6;
            uploadScratch.clear();
            uploadScratch.reserve(outVertexCount * 3);

            for (size_t i = 0; i + 1 < take; ++i) {
                const auto& a = cmd->vertices[offset + i];
                const auto& b = cmd->vertices[offset + i + 1];
                float dx = b.x - a.x;
                float dy = b.y - a.y;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len < 0.001f) continue;
                dx /= len;
                dy /= len;
                const float nx = -dy;
                const float ny = dx;
                const float halfWidthA = kBaseHalfWidth + 2.0f * a.extra;
                const float halfWidthB = kBaseHalfWidth + 2.0f * b.extra;

                const float aLx = a.x + nx * halfWidthA;
                const float aLy = a.y + ny * halfWidthA;
                const float aRx = a.x - nx * halfWidthA;
                const float aRy = a.y - ny * halfWidthA;
                const float bLx = b.x + nx * halfWidthB;
                const float bLy = b.y + ny * halfWidthB;
                const float bRx = b.x - nx * halfWidthB;
                const float bRy = b.y - ny * halfWidthB;

                const float cA = (float)a.colorArgb;
                const float cB = (float)b.colorArgb;
                uploadScratch.push_back(aLx); uploadScratch.push_back(aLy); uploadScratch.push_back(cA);
                uploadScratch.push_back(aRx); uploadScratch.push_back(aRy); uploadScratch.push_back(cA);
                uploadScratch.push_back(bLx); uploadScratch.push_back(bLy); uploadScratch.push_back(cB);
                uploadScratch.push_back(bLx); uploadScratch.push_back(bLy); uploadScratch.push_back(cB);
                uploadScratch.push_back(aRx); uploadScratch.push_back(aRy); uploadScratch.push_back(cA);
                uploadScratch.push_back(bRx); uploadScratch.push_back(bRy); uploadScratch.push_back(cB);
            }

            result.triangles += (uint32_t)(uploadScratch.size() / 9);
            const uint64_t upload = (uint64_t)result.uploadBytes + ((uint64_t)uploadScratch.size() * sizeof(float));
            result.uploadBytes = (upload > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)upload;
            offset += (take - 1);
        }
    }

    return result;
}

} // namespace detail

inline TrailGeometryPrepResult PreprocessTrailGeometry(
    const OverlayGpuCommandStream& stream) {
    std::vector<const OverlayGpuCommand*> trails{};
    trails.reserve(stream.Commands().size());
    uint64_t totalVertices = 0;
    for (const auto& cmd : stream.Commands()) {
        if (cmd.type != OverlayGpuCommandType::TrailPolyline || cmd.vertices.size() < 2) continue;
        trails.push_back(&cmd);
        totalVertices += cmd.vertices.size();
    }

    TrailGeometryPrepResult base{};
    constexpr uint32_t kBytesPerParticleSprite = 120; // 6 verts * (x,y,u,v,color) * 4 bytes
    for (const auto& cmd : stream.Commands()) {
        if (cmd.type != OverlayGpuCommandType::ParticleSprites || cmd.vertices.empty()) continue;
        ++base.particleBatches;
        base.particleSprites += (uint32_t)cmd.vertices.size();
        const uint64_t particleBytes = (uint64_t)base.particleUploadBytes +
                                       (uint64_t)cmd.vertices.size() * (uint64_t)kBytesPerParticleSprite;
        base.particleUploadBytes = (particleBytes > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)particleBytes;
    }
    const RippleGeometryPrepResult ripple = PreprocessRippleGeometry(stream);
    base.rippleBatches = ripple.batches;
    base.ripplePulses = ripple.pulses;
    base.rippleTriangles = ripple.triangles;
    base.rippleUploadBytes = ripple.uploadBytes;
    base.rippleBakedQuads = ripple.bakedQuads;
    base.rippleBakedVertices = ripple.bakedVertices;

    if (trails.empty()) return base;

    const unsigned hw = std::thread::hardware_concurrency();
    const uint32_t maxWorkers = (hw > 0) ? hw : 2;
    const uint32_t workerCount = (uint32_t)std::min<size_t>(trails.size(), maxWorkers);
    const bool shouldParallel = (workerCount >= 2 && totalVertices >= 2048);
    if (!shouldParallel) {
        TrailGeometryPrepResult out = detail::BuildTrailGeometryRange(trails, 0, trails.size());
        out.workers = 1;
        out.usedParallel = false;
        out.particleBatches = base.particleBatches;
        out.particleSprites = base.particleSprites;
        out.particleUploadBytes = base.particleUploadBytes;
        out.rippleBatches = base.rippleBatches;
        out.ripplePulses = base.ripplePulses;
        out.rippleTriangles = base.rippleTriangles;
        out.rippleUploadBytes = base.rippleUploadBytes;
        out.rippleBakedQuads = base.rippleBakedQuads;
        out.rippleBakedVertices = base.rippleBakedVertices;
        return out;
    }

    try {
        std::vector<std::future<TrailGeometryPrepResult>> futures{};
        futures.reserve(workerCount);
        size_t cursor = 0;
        const size_t chunk = (trails.size() + workerCount - 1) / workerCount;
        for (uint32_t i = 0; i < workerCount && cursor < trails.size(); ++i) {
            const size_t begin = cursor;
            const size_t end = std::min(trails.size(), begin + chunk);
            cursor = end;
            futures.push_back(std::async(std::launch::async, [begin, end, &trails]() {
                return detail::BuildTrailGeometryRange(trails, begin, end);
            }));
        }

        TrailGeometryPrepResult merged{};
        for (auto& f : futures) {
            merged = detail::MergeTrailPrep(merged, f.get());
        }
        merged.workers = (uint32_t)futures.size();
        merged.usedParallel = (futures.size() > 1);
        merged.particleBatches = base.particleBatches;
        merged.particleSprites = base.particleSprites;
        merged.particleUploadBytes = base.particleUploadBytes;
        merged.rippleBatches = base.rippleBatches;
        merged.ripplePulses = base.ripplePulses;
        merged.rippleTriangles = base.rippleTriangles;
        merged.rippleUploadBytes = base.rippleUploadBytes;
        merged.rippleBakedQuads = base.rippleBakedQuads;
        merged.rippleBakedVertices = base.rippleBakedVertices;
        return merged;
    } catch (...) {
        TrailGeometryPrepResult out = detail::BuildTrailGeometryRange(trails, 0, trails.size());
        out.workers = 1;
        out.usedParallel = false;
        out.particleBatches = base.particleBatches;
        out.particleSprites = base.particleSprites;
        out.particleUploadBytes = base.particleUploadBytes;
        out.rippleBatches = base.rippleBatches;
        out.ripplePulses = base.ripplePulses;
        out.rippleTriangles = base.rippleTriangles;
        out.rippleUploadBytes = base.rippleUploadBytes;
        out.rippleBakedQuads = base.rippleBakedQuads;
        out.rippleBakedVertices = base.rippleBakedVertices;
        return out;
    }
}

} // namespace mousefx::gpu
