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

#ifdef __cplusplus
}
#endif
