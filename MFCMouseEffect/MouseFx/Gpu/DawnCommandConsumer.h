#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

#include "MouseFx/Gpu/DawnOverlayBridge.h"
#include "MouseFx/Gpu/DawnRuntime.h"
#include "MouseFx/Gpu/DawnTrailGeometryPreprocessor.h"
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
    uint32_t preparedParticleBatches = 0;
    uint32_t preparedParticleSprites = 0;
    uint32_t preparedParticleUploadBytes = 0;
    uint32_t preparedRippleBatches = 0;
    uint32_t preparedRipplePulses = 0;
    uint32_t preparedRippleTriangles = 0;
    uint32_t preparedRippleUploadBytes = 0;
    uint32_t preprocessWorkers = 1;
    bool preprocessParallel = false;
    uint64_t noopSubmitAttempts = 0;
    uint64_t noopSubmitSuccess = 0;
    uint64_t emptyCommandSubmitAttempts = 0;
    uint64_t emptyCommandSubmitSuccess = 0;
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
    status.preparedParticleBatches = 0;
    status.preparedParticleSprites = 0;
    status.preparedParticleUploadBytes = 0;
    status.preparedRippleBatches = 0;
    status.preparedRipplePulses = 0;
    status.preparedRippleTriangles = 0;
    status.preparedRippleUploadBytes = 0;
    status.preprocessWorkers = 1;
    status.preprocessParallel = false;
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

    status.accepted = true;
    if (!runtime.queueReady) {
        // Queue is not ready yet: avoid CPU-heavy preprocessing while only diagnostics are active.
        // This keeps fallback behavior stable and reduces wasted work before real GPU submission is possible.
        status.detail = runtime.modernAbiDetected
            ? "accepted_waiting_queue_modern_abi_preprocess_skipped"
            : "accepted_waiting_queue_preprocess_skipped";
        ++status.acceptedFrames;
        return;
    }

    const TrailGeometryPrepResult prep = PreprocessTrailGeometry(stream);
    status.preparedTrailBatches = prep.batches;
    status.preparedTrailVertices = prep.vertices;
    status.preparedTrailSegments = prep.segments;
    status.preparedTrailTriangles = prep.triangles;
    status.preparedUploadBytes = prep.uploadBytes;
    status.preparedParticleBatches = prep.particleBatches;
    status.preparedParticleSprites = prep.particleSprites;
    status.preparedParticleUploadBytes = prep.particleUploadBytes;
    status.preparedRippleBatches = prep.rippleBatches;
    status.preparedRipplePulses = prep.ripplePulses;
    status.preparedRippleTriangles = prep.rippleTriangles;
    status.preparedRippleUploadBytes = prep.rippleUploadBytes;
    status.preprocessWorkers = prep.workers;
    status.preprocessParallel = prep.usedParallel;

    const bool hasTrailGeometry = (prep.triangles > 0);
    const bool hasRippleGeometry = (prep.rippleTriangles > 0);
    const bool hasParticleGeometry = (prep.particleSprites > 0);
    if (hasTrailGeometry || hasRippleGeometry || hasParticleGeometry) {
        ++status.noopSubmitAttempts;
        std::string submitDetail;
        if (TrySubmitNoopQueueWork(&submitDetail)) {
            ++status.noopSubmitSuccess;
            ++status.emptyCommandSubmitAttempts;
            std::string cmdSubmitDetail;
            if (TrySubmitEmptyCommandBuffer(&cmdSubmitDetail)) {
                ++status.emptyCommandSubmitSuccess;
                if (hasTrailGeometry) {
                    status.detail = prep.usedParallel
                        ? "accepted_trail_geometry_prepared_parallel_and_cmd_submit"
                        : "accepted_trail_geometry_prepared_and_cmd_submit";
                } else {
                    status.detail = prep.usedParallel
                        ? "accepted_nontrail_geometry_prepared_parallel_and_cmd_submit"
                        : "accepted_nontrail_geometry_prepared_and_cmd_submit";
                }
            } else {
                if (hasTrailGeometry) {
                    status.detail = prep.usedParallel
                        ? "accepted_trail_geometry_prepared_parallel_cmd_submit_pending"
                        : "accepted_trail_geometry_prepared_cmd_submit_pending";
                } else {
                    status.detail = prep.usedParallel
                        ? "accepted_nontrail_geometry_prepared_parallel_cmd_submit_pending"
                        : "accepted_nontrail_geometry_prepared_cmd_submit_pending";
                }
                if (!cmdSubmitDetail.empty()) {
                    status.detail += "_" + cmdSubmitDetail;
                }
            }
        } else {
            if (hasTrailGeometry) {
                status.detail = prep.usedParallel
                    ? "accepted_trail_geometry_prepared_parallel_submit_pending"
                    : "accepted_trail_geometry_prepared_submit_pending";
            } else {
                status.detail = prep.usedParallel
                    ? "accepted_nontrail_geometry_prepared_parallel_submit_pending"
                    : "accepted_nontrail_geometry_prepared_submit_pending";
            }
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
