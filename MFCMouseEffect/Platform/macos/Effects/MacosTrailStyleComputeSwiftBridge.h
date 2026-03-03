#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void mfx_compute_streamer_trail_segment_metrics_v1(
    double segmentRatio,
    double life,
    double headPower,
    double* outWidthPx,
    double* outCoreOpacity,
    double* outGlowOpacity);

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
    int* outForkSide);

void mfx_compute_meteor_trail_segment_metrics_v1(
    double segmentRatio,
    double life,
    double* outWidthPx,
    double* outTrailOpacity,
    int* outEmitCore,
    double* outCoreWidthPx,
    double* outCoreOpacity);

void mfx_compute_particle_trail_segment_metrics_v1(
    double segmentRatio,
    double life,
    double intensity,
    double* outRadiusPx,
    double* outOpacity,
    int* outEmitHalo,
    double* outHaloRadiusPx,
    double* outHaloOpacity);

void mfx_compute_tubes_node_render_metrics_v1(
    uint32_t chainIndex,
    uint32_t nodeIndex,
    uint32_t nodesCount,
    double fadeScale,
    double* outRadiusPx,
    double* outAmplitudePx,
    double* outAlpha,
    double* outNodePhase,
    double* outChainPhase);

void mfx_compute_tubes_head_follow_v1(
    double targetX,
    double targetY,
    double currentX,
    double currentY,
    double lag,
    double* outNextX,
    double* outNextY);

void mfx_compute_tubes_node_follow_v1(
    double prevX,
    double prevY,
    double currentX,
    double currentY,
    double lag,
    double minSegmentDistance,
    double* outNextX,
    double* outNextY);

double mfx_compute_trail_chromatic_hue_deg_v1(
    uint64_t nowMs,
    int32_t styleKind,
    uint32_t segmentIndex,
    uint32_t laneIndex);

#ifdef __cplusplus
}
#endif
