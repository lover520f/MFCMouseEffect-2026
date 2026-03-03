#include "pch.h"

#include "Platform/macos/Effects/MacosTrailStyleComputeSwiftBridge.h"

#include "MouseFx/Core/Effects/TrailStyleCompute.h"

namespace {

template <typename T>
void WriteValue(T* out, T value) {
    if (out != nullptr) {
        *out = value;
    }
}

} // namespace

void mfx_compute_streamer_trail_segment_metrics_v1(
    double segmentRatio,
    double life,
    double headPower,
    double* outWidthPx,
    double* outCoreOpacity,
    double* outGlowOpacity) {
    const mousefx::trail_style_compute::StreamerSegmentMetrics metrics =
        mousefx::trail_style_compute::ComputeStreamerSegmentMetrics(
            segmentRatio,
            life,
            headPower);
    WriteValue(outWidthPx, metrics.widthPx);
    WriteValue(outCoreOpacity, metrics.coreOpacity);
    WriteValue(outGlowOpacity, metrics.glowOpacity);
}

void mfx_compute_electric_trail_segment_metrics_v1(
    uint64_t frameBucket,
    uint32_t segmentIndex,
    double life,
    double lengthPx,
    double amplitudeScale,
    double forkChance,
    double* outJitterA,
    double* outJitterB,
    double* outGlowWidthPx,
    double* outCoreWidthPx,
    double* outGlowOpacity,
    double* outCoreOpacity,
    int* outEmitFork,
    double* outForkT,
    double* outForkLengthPx,
    double* outForkWidthPx,
    double* outForkOpacity,
    int* outForkSide) {
    const mousefx::trail_style_compute::ElectricSegmentMetrics metrics =
        mousefx::trail_style_compute::ComputeElectricSegmentMetrics(
            frameBucket,
            segmentIndex,
            life,
            lengthPx,
            amplitudeScale,
            forkChance);

    WriteValue(outJitterA, metrics.jitterA);
    WriteValue(outJitterB, metrics.jitterB);
    WriteValue(outGlowWidthPx, metrics.glowWidthPx);
    WriteValue(outCoreWidthPx, metrics.coreWidthPx);
    WriteValue(outGlowOpacity, metrics.glowOpacity);
    WriteValue(outCoreOpacity, metrics.coreOpacity);
    WriteValue(outEmitFork, metrics.emitFork ? 1 : 0);
    WriteValue(outForkT, metrics.forkT);
    WriteValue(outForkLengthPx, metrics.forkLengthPx);
    WriteValue(outForkWidthPx, metrics.forkWidthPx);
    WriteValue(outForkOpacity, metrics.forkOpacity);
    WriteValue(outForkSide, metrics.forkSide);
}

void mfx_compute_meteor_trail_segment_metrics_v1(
    double segmentRatio,
    double life,
    double* outWidthPx,
    double* outTrailOpacity,
    int* outEmitCore,
    double* outCoreWidthPx,
    double* outCoreOpacity) {
    const mousefx::trail_style_compute::MeteorSegmentMetrics metrics =
        mousefx::trail_style_compute::ComputeMeteorSegmentMetrics(
            segmentRatio,
            life);
    WriteValue(outWidthPx, metrics.widthPx);
    WriteValue(outTrailOpacity, metrics.trailOpacity);
    WriteValue(outEmitCore, metrics.emitCore ? 1 : 0);
    WriteValue(outCoreWidthPx, metrics.coreWidthPx);
    WriteValue(outCoreOpacity, metrics.coreOpacity);
}

void mfx_compute_particle_trail_segment_metrics_v1(
    double segmentRatio,
    double life,
    double intensity,
    double* outRadiusPx,
    double* outOpacity,
    int* outEmitHalo,
    double* outHaloRadiusPx,
    double* outHaloOpacity) {
    const mousefx::trail_style_compute::ParticleSegmentMetrics metrics =
        mousefx::trail_style_compute::ComputeParticleSegmentMetrics(
            segmentRatio,
            life,
            intensity);
    WriteValue(outRadiusPx, metrics.radiusPx);
    WriteValue(outOpacity, metrics.opacity);
    WriteValue(outEmitHalo, metrics.emitHalo ? 1 : 0);
    WriteValue(outHaloRadiusPx, metrics.haloRadiusPx);
    WriteValue(outHaloOpacity, metrics.haloOpacity);
}

void mfx_compute_tubes_node_render_metrics_v1(
    uint32_t chainIndex,
    uint32_t nodeIndex,
    uint32_t nodesCount,
    double fadeScale,
    double* outRadiusPx,
    double* outAmplitudePx,
    double* outAlpha,
    double* outNodePhase,
    double* outChainPhase) {
    const mousefx::trail_style_compute::TubesNodeRenderMetrics metrics =
        mousefx::trail_style_compute::ComputeTubesNodeRenderMetrics(
            chainIndex,
            nodeIndex,
            nodesCount,
            fadeScale);
    WriteValue(outRadiusPx, metrics.radiusPx);
    WriteValue(outAmplitudePx, metrics.amplitudePx);
    WriteValue(outAlpha, metrics.alpha);
    WriteValue(outNodePhase, metrics.nodePhase);
    WriteValue(outChainPhase, metrics.chainPhase);
}

void mfx_compute_tubes_head_follow_v1(
    double targetX,
    double targetY,
    double currentX,
    double currentY,
    double lag,
    double* outNextX,
    double* outNextY) {
    mousefx::trail_style_compute::ComputeTubesHeadFollow(
        targetX,
        targetY,
        currentX,
        currentY,
        lag,
        outNextX,
        outNextY);
}

void mfx_compute_tubes_node_follow_v1(
    double prevX,
    double prevY,
    double currentX,
    double currentY,
    double lag,
    double minSegmentDistance,
    double* outNextX,
    double* outNextY) {
    mousefx::trail_style_compute::ComputeTubesNodeFollow(
        prevX,
        prevY,
        currentX,
        currentY,
        lag,
        minSegmentDistance,
        outNextX,
        outNextY);
}

double mfx_compute_trail_chromatic_hue_deg_v1(
    uint64_t nowMs,
    int32_t styleKind,
    uint32_t segmentIndex,
    uint32_t laneIndex) {
    return mousefx::trail_style_compute::ComputeTrailChromaticHueDeg(
        nowMs,
        styleKind,
        segmentIndex,
        laneIndex);
}
