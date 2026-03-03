#pragma once

#include <cstdint>

namespace mousefx::trail_style_compute {

struct StreamerSegmentMetrics final {
    double widthPx = 0.0;
    double coreOpacity = 0.0;
    double glowOpacity = 0.0;
};

struct ElectricSegmentMetrics final {
    double jitterA = 0.0;
    double jitterB = 0.0;
    double glowWidthPx = 0.0;
    double coreWidthPx = 0.0;
    double glowOpacity = 0.0;
    double coreOpacity = 0.0;
    bool emitFork = false;
    double forkT = 0.0;
    double forkLengthPx = 0.0;
    double forkWidthPx = 0.0;
    double forkOpacity = 0.0;
    int forkSide = 1;
};

struct MeteorSegmentMetrics final {
    double widthPx = 0.0;
    double trailOpacity = 0.0;
    bool emitCore = false;
    double coreWidthPx = 0.0;
    double coreOpacity = 0.0;
};

struct ParticleSegmentMetrics final {
    double radiusPx = 0.0;
    double opacity = 0.0;
    bool emitHalo = false;
    double haloRadiusPx = 0.0;
    double haloOpacity = 0.0;
};

StreamerSegmentMetrics ComputeStreamerSegmentMetrics(
    double segmentRatio,
    double life,
    double headPower);

ElectricSegmentMetrics ComputeElectricSegmentMetrics(
    uint64_t frameBucket,
    uint32_t segmentIndex,
    double life,
    double lengthPx,
    double amplitudeScale,
    double forkChance);

MeteorSegmentMetrics ComputeMeteorSegmentMetrics(
    double segmentRatio,
    double life);

ParticleSegmentMetrics ComputeParticleSegmentMetrics(
    double segmentRatio,
    double life,
    double intensity);

} // namespace mousefx::trail_style_compute
