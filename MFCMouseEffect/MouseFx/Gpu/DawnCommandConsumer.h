#pragma once

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

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
    uint32_t rippleClickCommandCount = 0;
    uint32_t rippleHoverCommandCount = 0;
    uint32_t rippleHoldCommandCount = 0;
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
    uint32_t preparedRippleBakedQuads = 0;
    uint32_t preparedRippleBakedVertices = 0;
    uint32_t preprocessWorkers = 1;
    bool preprocessParallel = false;
    uint64_t noopSubmitAttempts = 0;
    uint64_t noopSubmitSuccess = 0;
    uint64_t emptyCommandSubmitAttempts = 0;
    uint64_t emptyCommandSubmitSuccess = 0;
    uint64_t nonTrailSubmitThrottled = 0;
    uint64_t trailPacketSubmitAttempts = 0;
    uint64_t trailPacketSubmitSuccess = 0;
    uint64_t ripplePacketSubmitAttempts = 0;
    uint64_t ripplePacketSubmitSuccess = 0;
    uint64_t rippleClickPacketSubmitAttempts = 0;
    uint64_t rippleClickPacketSubmitSuccess = 0;
    uint64_t rippleHoverPacketSubmitAttempts = 0;
    uint64_t rippleHoverPacketSubmitSuccess = 0;
    uint64_t rippleHoldPacketSubmitAttempts = 0;
    uint64_t rippleHoldPacketSubmitSuccess = 0;
    uint64_t particlePacketSubmitAttempts = 0;
    uint64_t particlePacketSubmitSuccess = 0;
    uint64_t mixedPacketSubmitAttempts = 0;
    uint64_t mixedPacketSubmitSuccess = 0;
    uint32_t passWarmupIndex = 0;
    uint32_t passWarmupTotal = 0;
    bool passWarmupDone = false;
    std::string passWarmupTag{};
    std::string detail = "not_submitted";
};

struct DawnCommandConsumeTimelinePoint {
    uint64_t submitTickMs = 0;
    uint32_t commandCount = 0;
    uint32_t trailCommandCount = 0;
    uint32_t rippleCommandCount = 0;
    uint32_t rippleClickCommandCount = 0;
    uint32_t rippleHoverCommandCount = 0;
    uint32_t rippleHoldCommandCount = 0;
    uint32_t particleCommandCount = 0;
    uint32_t preparedTrailTriangles = 0;
    uint32_t preparedRippleTriangles = 0;
    uint32_t preparedParticleSprites = 0;
    uint32_t preparedUploadBytes = 0;
    uint32_t preprocessWorkers = 1;
    bool preprocessParallel = false;
    uint32_t passWarmupIndex = 0;
    uint32_t passWarmupTotal = 0;
    bool passWarmupDone = false;
    std::string passWarmupTag{};
    std::string detail = "not_submitted";
};

inline constexpr size_t kDawnConsumerTimelineMax = 1800;

inline std::mutex& DawnConsumerMutex() {
    static std::mutex m;
    return m;
}

inline DawnCommandConsumeStatus& DawnConsumerStatusStorage() {
    static DawnCommandConsumeStatus status{};
    return status;
}

inline std::vector<DawnCommandConsumeTimelinePoint>& DawnConsumerTimelineStorage() {
    static std::vector<DawnCommandConsumeTimelinePoint> timeline{};
    return timeline;
}

inline void ResetDawnCommandConsumeStatus() {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    DawnConsumerStatusStorage() = DawnCommandConsumeStatus{};
    DawnConsumerTimelineStorage().clear();
}

inline void SubmitOverlayGpuCommands(
    const OverlayGpuCommandStream& stream,
    const DawnRuntimeStatus& runtime,
    const DawnOverlayBridgeStatus& bridge,
    const std::string& activeBackend,
    const std::string& pipelineMode) {
    DawnCommandConsumeStatus status{};
    {
        std::lock_guard<std::mutex> lock(DawnConsumerMutex());
        status = DawnConsumerStatusStorage();
    }
    auto publish = [&]() {
        std::lock_guard<std::mutex> lock(DawnConsumerMutex());
        DawnConsumerStatusStorage() = status;
        auto& timeline = DawnConsumerTimelineStorage();
        DawnCommandConsumeTimelinePoint point{};
        point.submitTickMs = status.submitTickMs;
        point.commandCount = status.commandCount;
        point.trailCommandCount = status.trailCommandCount;
        point.rippleCommandCount = status.rippleCommandCount;
        point.rippleClickCommandCount = status.rippleClickCommandCount;
        point.rippleHoverCommandCount = status.rippleHoverCommandCount;
        point.rippleHoldCommandCount = status.rippleHoldCommandCount;
        point.particleCommandCount = status.particleCommandCount;
        point.preparedTrailTriangles = status.preparedTrailTriangles;
        point.preparedRippleTriangles = status.preparedRippleTriangles;
        point.preparedParticleSprites = status.preparedParticleSprites;
        point.preparedUploadBytes = status.preparedUploadBytes;
        point.preprocessWorkers = status.preprocessWorkers;
        point.preprocessParallel = status.preprocessParallel;
        point.passWarmupIndex = status.passWarmupIndex;
        point.passWarmupTotal = status.passWarmupTotal;
        point.passWarmupDone = status.passWarmupDone;
        point.passWarmupTag = status.passWarmupTag;
        point.detail = status.detail;
        timeline.push_back(std::move(point));
        if (timeline.size() > kDawnConsumerTimelineMax) {
            timeline.erase(timeline.begin(), timeline.begin() + (timeline.size() - kDawnConsumerTimelineMax));
        }
    };

    status.submitTickMs = stream.FrameTickMs();
    status.commandCount = (uint32_t)stream.Commands().size();
    status.trailCommandCount = 0;
    status.rippleCommandCount = 0;
    status.rippleClickCommandCount = 0;
    status.rippleHoverCommandCount = 0;
    status.rippleHoldCommandCount = 0;
    status.particleCommandCount = 0;
    bool trailLatencyPriorityActive = false;
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
    status.preparedRippleBakedQuads = 0;
    status.preparedRippleBakedVertices = 0;
    status.preprocessWorkers = 1;
    status.preprocessParallel = false;
    for (const auto& cmd : stream.Commands()) {
        switch (cmd.type) {
        case OverlayGpuCommandType::TrailPolyline:
            ++status.trailCommandCount;
            if ((cmd.flags & OverlayGpuCommandFlags::kTrailLatencyPriority) != 0) {
                trailLatencyPriorityActive = true;
            }
            break;
        case OverlayGpuCommandType::RipplePulse:
            ++status.rippleCommandCount;
            if ((cmd.flags & OverlayGpuCommandFlags::kHoverContinuous) != 0) {
                ++status.rippleHoverCommandCount;
            } else if ((cmd.flags & OverlayGpuCommandFlags::kHoldContinuous) != 0) {
                ++status.rippleHoldCommandCount;
            } else {
                ++status.rippleClickCommandCount;
            }
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
    static std::atomic<int> s_prevBackendDawn{0};
    static std::atomic<int> s_prevRuntimeQueueReady{0};
    static std::atomic<uint64_t> s_dawnWarmupUntilTickMs{0};
    static std::atomic<uint32_t> s_dawnWarmupFramesLeft{0};
    static std::atomic<uint32_t> s_passWarmupIndex{0};
    static std::atomic<uint64_t> s_lastPassWarmupTickMs{0};
    static std::atomic<uint64_t> s_lastTrailSubmitWhenHoldTickMs{0};
    constexpr const char* kPassWarmupTags[] = {
        "trail_pass",
        "ripple_click_pass",
        "ripple_hover_pass",
        "ripple_hold_pass",
        "particle_pass",
        "mixed_pass",
    };
    constexpr uint32_t kPassWarmupCount = (uint32_t)(sizeof(kPassWarmupTags) / sizeof(kPassWarmupTags[0]));
    constexpr uint64_t kPassWarmupIntervalMs = 220;
    constexpr uint64_t kDawnWarmupDurationMs = 1800;
    constexpr uint32_t kDawnWarmupMaxFrames = 96;

    if (!backendDawn) {
        s_prevBackendDawn.store(0, std::memory_order_relaxed);
        s_prevRuntimeQueueReady.store(0, std::memory_order_relaxed);
        s_dawnWarmupUntilTickMs.store(0, std::memory_order_relaxed);
        s_dawnWarmupFramesLeft.store(0, std::memory_order_relaxed);
        s_passWarmupIndex.store(0, std::memory_order_relaxed);
        s_lastPassWarmupTickMs.store(0, std::memory_order_relaxed);
        s_lastTrailSubmitWhenHoldTickMs.store(0, std::memory_order_relaxed);
        status.accepted = false;
        status.detail = "backend_not_dawn";
        ++status.rejectedFrames;
        publish();
        return;
    }
    if (!compositorPath) {
        status.accepted = false;
        status.detail = "pipeline_not_compositor";
        ++status.rejectedFrames;
        publish();
        return;
    }
    if (!bridgeCompositor || !bridge.compositorApisReady) {
        status.accepted = false;
        status.detail = "bridge_not_ready";
        ++status.rejectedFrames;
        publish();
        return;
    }
    if (!runtimeReady) {
        status.accepted = false;
        status.detail = "runtime_not_ready";
        ++status.rejectedFrames;
        publish();
        return;
    }

    const int previousBackendDawn = s_prevBackendDawn.exchange(1, std::memory_order_relaxed);
    const int currentQueueReady = runtime.queueReady ? 1 : 0;
    const int previousQueueReady = s_prevRuntimeQueueReady.exchange(currentQueueReady, std::memory_order_relaxed);
    const bool queueBecameReady = (currentQueueReady == 1) && (previousQueueReady == 0);
    if (previousBackendDawn == 0 || queueBecameReady) {
        const uint64_t nowTick = (status.submitTickMs > 0) ? status.submitTickMs : GetTickCount64();
        s_dawnWarmupUntilTickMs.store(nowTick + kDawnWarmupDurationMs, std::memory_order_relaxed);
        s_dawnWarmupFramesLeft.store(kDawnWarmupMaxFrames, std::memory_order_relaxed);
        s_passWarmupIndex.store(0, std::memory_order_relaxed);
        s_lastPassWarmupTickMs.store(nowTick, std::memory_order_relaxed);
        s_lastTrailSubmitWhenHoldTickMs.store(0, std::memory_order_relaxed);
    }

    {
        const uint32_t passIndex = s_passWarmupIndex.load(std::memory_order_relaxed);
        status.passWarmupIndex = (passIndex > kPassWarmupCount) ? kPassWarmupCount : passIndex;
        status.passWarmupTotal = kPassWarmupCount;
        status.passWarmupDone = (status.passWarmupIndex >= kPassWarmupCount);
        if (!status.passWarmupDone && status.passWarmupIndex < kPassWarmupCount) {
            status.passWarmupTag = kPassWarmupTags[status.passWarmupIndex];
        } else {
            status.passWarmupTag.clear();
        }
    }

    if (status.commandCount == 0) {
        status.accepted = true;
        status.detail = "accepted_empty_frame";
        ++status.acceptedFrames;
        publish();
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
        publish();
        return;
    }

    // hold_neon3d path may not emit ripple-hold commands; use trail latency-priority flag as hold signal too.
    const bool holdActive = (status.rippleHoldCommandCount > 0) || trailLatencyPriorityActive;

    if (!status.passWarmupDone) {
        const uint64_t nowTick = (status.submitTickMs > 0) ? status.submitTickMs : GetTickCount64();
        const uint64_t lastTick = s_lastPassWarmupTickMs.load(std::memory_order_relaxed);
        if (lastTick == 0 || nowTick >= lastTick + kPassWarmupIntervalMs) {
            const uint32_t passIndex = s_passWarmupIndex.load(std::memory_order_relaxed);
            if (passIndex < kPassWarmupCount) {
                std::string warmDetail;
                if (TrySubmitEmptyCommandBufferTagged(kPassWarmupTags[passIndex], &warmDetail)) {
                    s_passWarmupIndex.store(passIndex + 1, std::memory_order_relaxed);
                    status.passWarmupIndex = passIndex + 1;
                    status.passWarmupDone = (status.passWarmupIndex >= kPassWarmupCount);
                    status.passWarmupTag = status.passWarmupDone ? "" : kPassWarmupTags[status.passWarmupIndex];
                }
                s_lastPassWarmupTickMs.store(nowTick, std::memory_order_relaxed);
            }
        }
    }

    const uint64_t warmupUntilTick = s_dawnWarmupUntilTickMs.load(std::memory_order_relaxed);
    uint32_t warmupFramesLeft = s_dawnWarmupFramesLeft.load(std::memory_order_relaxed);
    const bool warmupByTime = (status.submitTickMs > 0) && (warmupUntilTick > 0) && (status.submitTickMs < warmupUntilTick);
    const bool warmupByFrames = (warmupFramesLeft > 0);
    if ((warmupByTime || warmupByFrames) && status.trailCommandCount > 0 && !holdActive) {
        if (warmupFramesLeft > 0) {
            s_dawnWarmupFramesLeft.store(warmupFramesLeft - 1, std::memory_order_relaxed);
        }
        ++status.noopSubmitAttempts;
        std::string warmupSubmitDetail;
        if (TrySubmitNoopQueueWork(&warmupSubmitDetail)) {
            ++status.noopSubmitSuccess;
            status.detail = "accepted_queue_ready_warmup_trail_preprocess_skipped";
        } else {
            status.detail = "accepted_queue_ready_warmup_trail_preprocess_skipped_submit_pending";
            if (!warmupSubmitDetail.empty()) {
                status.detail += "_" + warmupSubmitDetail;
            }
        }
        ++status.acceptedFrames;
        publish();
        return;
    }

    bool skipTrailGeometryBuild = false;
    if (holdActive && status.trailCommandCount > 0) {
        constexpr uint64_t kTrailSubmitIntervalWhenHoldMs = 8;
        const uint64_t nowTick = (status.submitTickMs > 0) ? status.submitTickMs : GetTickCount64();
        const uint64_t lastTrailTick = s_lastTrailSubmitWhenHoldTickMs.load(std::memory_order_relaxed);
        if (lastTrailTick > 0 && (nowTick - lastTrailTick) < kTrailSubmitIntervalWhenHoldMs) {
            skipTrailGeometryBuild = true;
        }
    }

    constexpr size_t kHoldTrailVertexCapPerCommand = 24;
    const size_t trailVertexCapPerCommand = holdActive ? kHoldTrailVertexCapPerCommand : 0;
    const TrailGeometryPrepResult prep = PreprocessTrailGeometry(
        stream,
        skipTrailGeometryBuild,
        trailVertexCapPerCommand);
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
    status.preparedRippleBakedQuads = prep.rippleBakedQuads;
    status.preparedRippleBakedVertices = prep.rippleBakedVertices;
    status.preprocessWorkers = prep.workers;
    status.preprocessParallel = prep.usedParallel;

    const bool hasTrailGeometry = (prep.triangles > 0);
    const bool hasRippleGeometry = (prep.rippleTriangles > 0);
    const bool hasParticleGeometry = (prep.particleSprites > 0);
    const bool hasOnlyNonTrailGeometry = (!hasTrailGeometry) && (hasRippleGeometry || hasParticleGeometry);
    const bool nonTrailMixed = hasRippleGeometry && hasParticleGeometry;
    const bool nonTrailRippleOnly = hasRippleGeometry && !hasParticleGeometry;
    const bool nonTrailParticleOnly = hasParticleGeometry && !hasRippleGeometry;
    auto rippleModeSuffix = [&]() -> const char* {
        if (status.rippleHoverCommandCount > 0 &&
            status.rippleHoldCommandCount == 0 &&
            status.rippleClickCommandCount == 0) {
            return "_hover";
        }
        if (status.rippleHoldCommandCount > 0 &&
            status.rippleHoverCommandCount == 0 &&
            status.rippleClickCommandCount == 0) {
            return "_hold";
        }
        if (status.rippleClickCommandCount > 0 &&
            status.rippleHoverCommandCount == 0 &&
            status.rippleHoldCommandCount == 0) {
            return "_click";
        }
        if (status.rippleCommandCount > 0) {
            return "_mixed";
        }
        return "";
    };
    auto nonTrailDetail = [&](const char* base) -> std::string {
        if (nonTrailRippleOnly) {
            std::string out = std::string(base) + "_ripple";
            out += rippleModeSuffix();
            return out;
        }
        if (nonTrailParticleOnly) return std::string(base) + "_particle";
        if (nonTrailMixed) return std::string(base) + "_mixed";
        return std::string(base);
    };
    static std::atomic<uint64_t> s_lastNonTrailSubmitTickMs{0};
    static std::atomic<uint64_t> s_lastHoldMixedNonTrailSubmitTickMs{0};
    constexpr uint64_t kNonTrailSubmitIntervalMs = 8; // cap non-trail submit to ~120Hz
    constexpr uint64_t kNonTrailSubmitIntervalWhenHoldMs = 2; // hold path prefers latency while still rate-limited
    constexpr uint64_t kHoldMixedNonTrailSubmitIntervalMs = 8; // baseline hold mixed-frame non-trail pacing
    constexpr uint64_t kHoldMixedNonTrailSubmitIntervalHeavyMs = 16; // heavier trail load favors trail continuity
    constexpr uint32_t kHoldMixedTrailHeavyTriangleThreshold = 6;
    if (hasTrailGeometry || hasRippleGeometry || hasParticleGeometry) {
        if (hasOnlyNonTrailGeometry && status.submitTickMs > 0) {
            const uint64_t nonTrailIntervalMs = holdActive
                ? kNonTrailSubmitIntervalWhenHoldMs
                : kNonTrailSubmitIntervalMs;
            const uint64_t lastNonTrailTick = s_lastNonTrailSubmitTickMs.load(std::memory_order_relaxed);
            if (lastNonTrailTick > 0 &&
                status.submitTickMs - lastNonTrailTick < nonTrailIntervalMs) {
                ++status.nonTrailSubmitThrottled;
                status.detail = nonTrailDetail(holdActive
                    ? "accepted_nontrail_geometry_submit_throttled_hold_priority"
                    : "accepted_nontrail_geometry_submit_throttled");
                ++status.acceptedFrames;
                publish();
                return;
            }
            s_lastNonTrailSubmitTickMs.store(status.submitTickMs, std::memory_order_relaxed);
        }
        bool submitNonTrailWithTrail = true;
        bool holdMixedHeavyTrail = false;
        if (holdActive && hasTrailGeometry && (hasRippleGeometry || hasParticleGeometry) && status.submitTickMs > 0) {
            holdMixedHeavyTrail = (prep.triangles >= kHoldMixedTrailHeavyTriangleThreshold);
            const uint64_t holdMixedNonTrailIntervalMs =
                holdMixedHeavyTrail
                ? kHoldMixedNonTrailSubmitIntervalHeavyMs
                : kHoldMixedNonTrailSubmitIntervalMs;
            const uint64_t lastHoldMixedTick =
                s_lastHoldMixedNonTrailSubmitTickMs.load(std::memory_order_relaxed);
            if (lastHoldMixedTick > 0 &&
                status.submitTickMs - lastHoldMixedTick < holdMixedNonTrailIntervalMs) {
                submitNonTrailWithTrail = false;
                ++status.nonTrailSubmitThrottled;
            } else {
                s_lastHoldMixedNonTrailSubmitTickMs.store(status.submitTickMs, std::memory_order_relaxed);
            }
        }

        ++status.emptyCommandSubmitAttempts;
        std::string cmdSubmitDetail;
        bool cmdSubmitOk = false;
        auto submitNonTrailPacket = [&](std::string* detailOut) -> bool {
            std::string localDetail;
            bool localOk = false;
            if (nonTrailRippleOnly && prep.rippleBakedVertices > 0) {
                ++status.ripplePacketSubmitAttempts;
                const bool clickOnly = status.rippleClickCommandCount > 0 &&
                    status.rippleHoverCommandCount == 0 &&
                    status.rippleHoldCommandCount == 0;
                const bool hoverOnly = status.rippleHoverCommandCount > 0 &&
                    status.rippleClickCommandCount == 0 &&
                    status.rippleHoldCommandCount == 0;
                const bool holdOnly = status.rippleHoldCommandCount > 0 &&
                    status.rippleClickCommandCount == 0 &&
                    status.rippleHoverCommandCount == 0;
                if (clickOnly) {
                    ++status.rippleClickPacketSubmitAttempts;
                    localOk = TrySubmitRippleClickBakedPacket(prep.rippleBakedVertices, prep.rippleUploadBytes, &localDetail);
                    if (localOk) ++status.rippleClickPacketSubmitSuccess;
                } else if (hoverOnly) {
                    ++status.rippleHoverPacketSubmitAttempts;
                    localOk = TrySubmitRippleHoverBakedPacket(prep.rippleBakedVertices, prep.rippleUploadBytes, &localDetail);
                    if (localOk) ++status.rippleHoverPacketSubmitSuccess;
                } else if (holdOnly) {
                    ++status.rippleHoldPacketSubmitAttempts;
                    localOk = TrySubmitRippleHoldBakedPacket(prep.rippleBakedVertices, prep.rippleUploadBytes, &localDetail);
                    if (localOk) ++status.rippleHoldPacketSubmitSuccess;
                } else {
                    localOk = TrySubmitRippleBakedPacket(prep.rippleBakedVertices, prep.rippleUploadBytes, &localDetail);
                }
                if (localOk) ++status.ripplePacketSubmitSuccess;
            } else if (nonTrailParticleOnly && prep.particleSprites > 0) {
                ++status.particlePacketSubmitAttempts;
                localOk = TrySubmitParticleBakedPacket(prep.particleSprites, prep.particleUploadBytes, &localDetail);
                if (localOk) ++status.particlePacketSubmitSuccess;
            } else if (nonTrailMixed && (prep.rippleBakedVertices > 0 || prep.particleSprites > 0)) {
                ++status.mixedPacketSubmitAttempts;
                const uint32_t mixedUploadBytes = prep.rippleUploadBytes + prep.particleUploadBytes;
                localOk = TrySubmitMixedBakedPacket(
                    prep.rippleBakedVertices,
                    prep.particleSprites,
                    mixedUploadBytes,
                    &localDetail);
                if (localOk) ++status.mixedPacketSubmitSuccess;
            } else {
                localOk = TrySubmitEmptyCommandBufferTagged("nontrail", &localDetail);
            }
            if (detailOut) *detailOut = localDetail;
            return localOk;
        };

        if (hasTrailGeometry && prep.vertices > 0) {
            std::string nonTrailDetailOnly;
            bool nonTrailOk = true;
            const bool hasNonTrailGeometry = (hasRippleGeometry || hasParticleGeometry);
            bool trailOk = true;
            std::string trailDetail;
            if (holdActive && hasNonTrailGeometry) {
                if (submitNonTrailWithTrail) {
                    nonTrailOk = submitNonTrailPacket(&nonTrailDetailOnly);
                } else {
                    nonTrailDetailOnly = holdMixedHeavyTrail
                        ? "nontrail_submit_skipped_hold_priority_heavy_trail"
                        : "nontrail_submit_skipped_hold_priority";
                }
                ++status.trailPacketSubmitAttempts;
                trailOk = TrySubmitTrailBakedPacket(prep.vertices, prep.uploadBytes, &trailDetail);
                if (trailOk) {
                    ++status.trailPacketSubmitSuccess;
                    if (status.submitTickMs > 0) {
                        s_lastTrailSubmitWhenHoldTickMs.store(status.submitTickMs, std::memory_order_relaxed);
                    }
                }
            } else {
                ++status.trailPacketSubmitAttempts;
                trailOk = TrySubmitTrailBakedPacket(prep.vertices, prep.uploadBytes, &trailDetail);
                if (trailOk) {
                    ++status.trailPacketSubmitSuccess;
                    if (holdActive && status.submitTickMs > 0) {
                        s_lastTrailSubmitWhenHoldTickMs.store(status.submitTickMs, std::memory_order_relaxed);
                    }
                }
            }
            if (!holdActive && hasNonTrailGeometry) {
                nonTrailOk = submitNonTrailPacket(&nonTrailDetailOnly);
            }
            cmdSubmitOk = trailOk && nonTrailOk;
            cmdSubmitDetail.clear();
            if (holdActive && hasNonTrailGeometry) {
                cmdSubmitDetail = nonTrailDetailOnly;
                if (!trailDetail.empty()) {
                    if (!cmdSubmitDetail.empty()) cmdSubmitDetail += "|";
                    cmdSubmitDetail += trailDetail;
                }
            } else {
                cmdSubmitDetail = trailDetail;
                if (!nonTrailDetailOnly.empty()) {
                    if (!cmdSubmitDetail.empty()) cmdSubmitDetail += "|";
                    cmdSubmitDetail += nonTrailDetailOnly;
                }
            }
        } else if (!hasTrailGeometry && (hasRippleGeometry || hasParticleGeometry)) {
            cmdSubmitOk = submitNonTrailPacket(&cmdSubmitDetail);
        } else {
            const char* submitTag = hasTrailGeometry ? "trail" : (nonTrailRippleOnly ? "ripple" : (nonTrailParticleOnly ? "particle" : "mixed"));
            cmdSubmitOk = TrySubmitEmptyCommandBufferTagged(submitTag, &cmdSubmitDetail);
        }
        if (cmdSubmitOk) {
            ++status.emptyCommandSubmitSuccess;
            if (hasTrailGeometry && (hasRippleGeometry || hasParticleGeometry)) {
                status.detail = prep.usedParallel
                    ? "accepted_trail_and_nontrail_geometry_prepared_parallel_and_cmd_submit"
                    : "accepted_trail_and_nontrail_geometry_prepared_and_cmd_submit";
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("trail_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_trail_baked_packet";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_click_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_packet_click";
                } else if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_hover_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_packet_hover";
                } else if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_hold_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_packet_hold";
                } else if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_packet";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("particle_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_particle_packet";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("mixed_packet_submit_ok_") != std::string::npos) {
                    status.detail += "_mixed_baked_packet";
                }
            } else if (hasTrailGeometry) {
                status.detail = prep.usedParallel
                    ? "accepted_trail_geometry_prepared_parallel_and_cmd_submit"
                    : "accepted_trail_geometry_prepared_and_cmd_submit";
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("trail_packet_submit_ok_") == 0) {
                    status.detail += "_trail_baked_packet";
                }
            } else {
                const bool rippleBakedReady = (prep.rippleBakedVertices > 0);
                const bool particleBakedReady = (prep.particleSprites > 0 && nonTrailParticleOnly);
                status.detail = prep.usedParallel
                    ? nonTrailDetail("accepted_nontrail_geometry_prepared_parallel_and_cmd_submit")
                    : nonTrailDetail("accepted_nontrail_geometry_prepared_and_cmd_submit");
                if (rippleBakedReady) status.detail += "_ripple_baked";
                if (particleBakedReady) status.detail += "_particle_baked";
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_packet_submit_ok_") == 0) {
                    status.detail += "_packet";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_click_packet_submit_ok_") == 0) {
                    status.detail += "_packet_click";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_hover_packet_submit_ok_") == 0) {
                    status.detail += "_packet_hover";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("ripple_hold_packet_submit_ok_") == 0) {
                    status.detail += "_packet_hold";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("particle_packet_submit_ok_") == 0) {
                    status.detail += "_packet";
                }
                if (!cmdSubmitDetail.empty() && cmdSubmitDetail.find("mixed_packet_submit_ok_") == 0) {
                    status.detail += "_mixed_baked_packet";
                }
            }
        } else {
            if (hasTrailGeometry && (hasRippleGeometry || hasParticleGeometry)) {
                status.detail = prep.usedParallel
                    ? "accepted_trail_and_nontrail_geometry_prepared_parallel_cmd_submit_pending"
                    : "accepted_trail_and_nontrail_geometry_prepared_cmd_submit_pending";
            } else if (hasTrailGeometry) {
                status.detail = prep.usedParallel
                    ? "accepted_trail_geometry_prepared_parallel_cmd_submit_pending"
                    : "accepted_trail_geometry_prepared_cmd_submit_pending";
            } else {
                status.detail = prep.usedParallel
                    ? nonTrailDetail("accepted_nontrail_geometry_prepared_parallel_cmd_submit_pending")
                    : nonTrailDetail("accepted_nontrail_geometry_prepared_cmd_submit_pending");
            }
            if (!cmdSubmitDetail.empty()) {
                status.detail += "_" + cmdSubmitDetail;
            }
        }
    } else {
        status.detail = "accepted_no_trail_geometry";
    }
    ++status.acceptedFrames;
    publish();
}

inline DawnCommandConsumeStatus GetDawnCommandConsumeStatus() {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    return DawnConsumerStatusStorage();
}

inline std::vector<DawnCommandConsumeTimelinePoint> GetDawnCommandConsumeTimeline() {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    return DawnConsumerTimelineStorage();
}

inline std::vector<DawnCommandConsumeTimelinePoint> GetDawnCommandConsumeTimelineTail(size_t maxPoints) {
    std::lock_guard<std::mutex> lock(DawnConsumerMutex());
    const auto& timeline = DawnConsumerTimelineStorage();
    if (maxPoints == 0 || timeline.empty()) {
        return {};
    }
    if (timeline.size() <= maxPoints) {
        return timeline;
    }
    const size_t begin = timeline.size() - maxPoints;
    return std::vector<DawnCommandConsumeTimelinePoint>(timeline.begin() + begin, timeline.end());
}

} // namespace mousefx::gpu
