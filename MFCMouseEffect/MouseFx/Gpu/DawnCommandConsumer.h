#pragma once

#include <cstddef>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "MouseFx/Gpu/DawnOverlayBridge.h"
#include "MouseFx/Gpu/DawnRuntime.h"
#include "MouseFx/Gpu/OverlayGpuCommandStream.h"

namespace mousefx::gpu {

struct DawnCommandConsumeStatus {
    uint64_t submitTickMs = 0;
    uint32_t commandCount = 0;
    uint32_t trailCommandCount = 0;
    uint32_t rippleCommandCount = 0;
    uint32_t particleCommandCount = 0;
    bool accepted = false;
    uint64_t acceptedFrames = 0;
    uint64_t rejectedFrames = 0;
    uint32_t preparedTrailBatches = 0;
    uint32_t preparedTrailVertices = 0;
    uint32_t preparedTrailSegments = 0;
    uint32_t preparedTrailTriangles = 0;
    uint32_t preparedUploadBytes = 0;
    uint64_t noopSubmitAttempts = 0;
    uint64_t noopSubmitSuccess = 0;
    std::string detail = "not_submitted";
};

inline std::mutex& DawnConsumerMutex() {
    static std::mutex m;
    return m;
}

inline DawnCommandConsumeStatus& DawnConsumerStatusStorage() {
    static DawnCommandConsumeStatus status{};
    return status;
}

inline void ResetDawnCommandConsumeStatus() {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    DawnConsumerStatusStorage() = DawnCommandConsumeStatus{};
}

inline void SubmitOverlayGpuCommands(
    const OverlayGpuCommandStream& stream,
    const DawnRuntimeStatus& runtime,
    const DawnOverlayBridgeStatus& bridge,
    const std::string& activeBackend,
    const std::string& pipelineMode) {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    DawnCommandConsumeStatus& status = DawnConsumerStatusStorage();

    status.submitTickMs = stream.FrameTickMs();
    status.commandCount = (uint32_t)stream.Commands().size();
    status.trailCommandCount = 0;
    status.rippleCommandCount = 0;
    status.particleCommandCount = 0;
    status.preparedTrailBatches = 0;
    status.preparedTrailVertices = 0;
    status.preparedTrailSegments = 0;
    status.preparedTrailTriangles = 0;
    status.preparedUploadBytes = 0;
    for (const auto& cmd : stream.Commands()) {
        switch (cmd.type) {
        case OverlayGpuCommandType::TrailPolyline:
            ++status.trailCommandCount;
            break;
        case OverlayGpuCommandType::RipplePulse:
            ++status.rippleCommandCount;
            break;
        case OverlayGpuCommandType::ParticleSprites:
            ++status.particleCommandCount;
            break;
        default:
            break;
        }
    }

    const bool backendDawn = (activeBackend == "dawn");
    const bool bridgeCompositor = (bridge.mode == "compositor");
    const bool runtimeReady = runtime.probe.moduleLoaded && runtime.probe.canCreateInstance;
    const bool compositorPath = (pipelineMode == "dawn_compositor");

    if (!backendDawn) {
        status.accepted = false;
        status.detail = "backend_not_dawn";
        ++status.rejectedFrames;
        return;
    }
    if (!compositorPath) {
        status.accepted = false;
        status.detail = "pipeline_not_compositor";
        ++status.rejectedFrames;
        return;
    }
    if (!bridgeCompositor || !bridge.compositorApisReady) {
        status.accepted = false;
        status.detail = "bridge_not_ready";
        ++status.rejectedFrames;
        return;
    }
    if (!runtimeReady) {
        status.accepted = false;
        status.detail = "runtime_not_ready";
        ++status.rejectedFrames;
        return;
    }

    if (status.commandCount == 0) {
        status.accepted = true;
        status.detail = "accepted_empty_frame";
        ++status.acceptedFrames;
        return;
    }

    // Stage 37: build GPU-drawable trail triangle geometry.
    // No real DrawPass submission yet, but the generated triangles are upload-ready.
    uint32_t preparedBatches = 0;
    uint32_t preparedVertices = 0;
    uint32_t preparedSegments = 0;
    uint32_t preparedTriangles = 0;
    uint64_t uploadBytes = 0;
    std::vector<float> uploadScratch{};
    constexpr size_t kMaxTrailVerticesPerBatch = 1024;
    constexpr float kBaseHalfWidth = 2.2f;
    for (const auto& cmd : stream.Commands()) {
        if (cmd.type != OverlayGpuCommandType::TrailPolyline) continue;
        if (cmd.vertices.size() < 2) continue;
        size_t begin = 0;
        while (begin + 1 < cmd.vertices.size()) {
            const size_t remain = cmd.vertices.size() - begin;
            const size_t take = (remain > kMaxTrailVerticesPerBatch) ? kMaxTrailVerticesPerBatch : remain;
            if (take < 2) break;
            ++preparedBatches;
            preparedVertices += (uint32_t)take;
            preparedSegments += (uint32_t)(take - 1);

            // Convert polyline into a simple quad-strip (2 triangles per segment).
            // Packed vertex layout: x, y, rgba_u32_as_f32.
            const size_t localSegments = take - 1;
            const size_t outVertexCount = localSegments * 6; // 2 triangles per segment
            uploadScratch.clear();
            uploadScratch.reserve(outVertexCount * 3);
            for (size_t i = 0; i + 1 < take; ++i) {
                const auto& a = cmd.vertices[begin + i];
                const auto& b = cmd.vertices[begin + i + 1];
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
                // tri0: aL, aR, bL
                uploadScratch.push_back(aLx); uploadScratch.push_back(aLy); uploadScratch.push_back(cA);
                uploadScratch.push_back(aRx); uploadScratch.push_back(aRy); uploadScratch.push_back(cA);
                uploadScratch.push_back(bLx); uploadScratch.push_back(bLy); uploadScratch.push_back(cB);
                // tri1: bL, aR, bR
                uploadScratch.push_back(bLx); uploadScratch.push_back(bLy); uploadScratch.push_back(cB);
                uploadScratch.push_back(aRx); uploadScratch.push_back(aRy); uploadScratch.push_back(cA);
                uploadScratch.push_back(bRx); uploadScratch.push_back(bRy); uploadScratch.push_back(cB);
            }
            const uint32_t triangles = (uint32_t)(uploadScratch.size() / 9); // 3 floats per vertex, 3 vertices per tri
            preparedTriangles += triangles;
            uploadBytes += (uint64_t)uploadScratch.size() * sizeof(float);
            begin += (take - 1); // keep one overlap vertex between batches
        }
    }
    status.preparedTrailBatches = preparedBatches;
    status.preparedTrailVertices = preparedVertices;
    status.preparedTrailSegments = preparedSegments;
    status.preparedTrailTriangles = preparedTriangles;
    status.preparedUploadBytes = (uploadBytes > 0xFFFFFFFFull) ? 0xFFFFFFFFu : (uint32_t)uploadBytes;

    status.accepted = true;
    if (preparedTriangles > 0) {
        ++status.noopSubmitAttempts;
        std::string submitDetail;
        if (TrySubmitNoopQueueWork(&submitDetail)) {
            ++status.noopSubmitSuccess;
            status.detail = "accepted_trail_geometry_prepared_and_submitted";
        } else {
            status.detail = "accepted_trail_geometry_prepared_submit_pending";
            if (!submitDetail.empty()) {
                status.detail += "_" + submitDetail;
            }
        }
    } else {
        status.detail = "accepted_no_trail_geometry";
    }
    ++status.acceptedFrames;
}

inline DawnCommandConsumeStatus GetDawnCommandConsumeStatus() {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    return DawnConsumerStatusStorage();
}

} // namespace mousefx::gpu
