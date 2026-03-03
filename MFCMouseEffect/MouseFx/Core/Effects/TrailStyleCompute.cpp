#include "pch.h"

#include "MouseFx/Core/Effects/TrailStyleCompute.h"

#include <algorithm>
#include <cmath>

namespace mousefx::trail_style_compute {
namespace {

double Clamp01(double value) {
    return std::clamp(value, 0.0, 1.0);
}

double ClampDouble(double value, double minValue, double maxValue) {
    return std::clamp(value, minValue, maxValue);
}

uint32_t XorShift32(uint32_t* state) {
    if (state == nullptr) {
        return 0;
    }
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

double Next01(uint32_t* state) {
    return static_cast<double>(XorShift32(state)) / static_cast<double>(UINT32_MAX);
}

double NextRange(uint32_t* state, double minValue, double maxValue) {
    const double t = Next01(state);
    return minValue + (maxValue - minValue) * t;
}

} // namespace

StreamerSegmentMetrics ComputeStreamerSegmentMetrics(
    double segmentRatio,
    double life,
    double headPower) {
    StreamerSegmentMetrics metrics{};
    const double safeRatio = Clamp01(segmentRatio);
    const double safeLife = Clamp01(life);
    const double safeHeadPower = ClampDouble(headPower, 0.8, 3.0);
    const double head = std::pow(safeRatio, safeHeadPower);
    metrics.widthPx = std::max(0.0, 2.0 + 18.0 * head * safeLife);
    metrics.coreOpacity = Clamp01((220.0 * head * safeLife) / 255.0);
    metrics.glowOpacity = Clamp01((90.0 * head * safeLife) / 255.0);
    return metrics;
}

ElectricSegmentMetrics ComputeElectricSegmentMetrics(
    uint64_t frameBucket,
    uint32_t segmentIndex,
    double life,
    double lengthPx,
    double amplitudeScale,
    double forkChance) {
    ElectricSegmentMetrics metrics{};
    const double safeLife = Clamp01(life);
    const double safeLength = std::max(0.0, lengthPx);
    const double safeAmp = ClampDouble(amplitudeScale, 0.2, 3.0);
    const double safeFork = ClampDouble(forkChance, 0.0, 0.5);

    uint32_t seed = static_cast<uint32_t>((frameBucket & 0xFFFFFFFFu) ^
                                          (static_cast<uint64_t>(segmentIndex) * 0x9E3779B9u));
    if (seed == 0) {
        seed = 0xA341316Cu;
    }

    const double amp = ClampDouble(safeLength * 0.12, 2.0, 10.0) * safeLife * safeAmp;
    metrics.jitterA = NextRange(&seed, -1.0, 1.0) * amp;
    metrics.jitterB = NextRange(&seed, -1.0, 1.0) * amp;
    metrics.coreWidthPx = std::max(1.0, 2.2 * safeLife);
    metrics.glowWidthPx = metrics.coreWidthPx * 3.0;
    metrics.coreOpacity = safeLife;
    metrics.glowOpacity = Clamp01(safeLife * 0.5);

    const double dynamicForkChance = safeFork * safeLife;
    if (Next01(&seed) < dynamicForkChance) {
        metrics.emitFork = true;
        metrics.forkT = NextRange(&seed, 0.35, 0.75);
        metrics.forkLengthPx = NextRange(&seed, 10.0, 22.0) * safeLife;
        metrics.forkWidthPx = std::max(1.0, metrics.coreWidthPx * 1.2);
        metrics.forkOpacity = Clamp01(safeLife * 0.8);
        metrics.forkSide = (Next01(&seed) < 0.5) ? -1 : 1;
    }

    return metrics;
}

MeteorSegmentMetrics ComputeMeteorSegmentMetrics(
    double segmentRatio,
    double life) {
    MeteorSegmentMetrics metrics{};
    const double safeRatio = Clamp01(segmentRatio);
    const double safeLife = Clamp01(life);
    metrics.widthPx = std::max(0.0, 1.0 + 12.0 * safeRatio * safeLife);
    metrics.trailOpacity = Clamp01((180.0 * safeRatio * safeLife) / 255.0);
    if (safeRatio > 0.6) {
        metrics.emitCore = true;
        metrics.coreWidthPx = std::max(1.0, metrics.widthPx * 0.3);
        metrics.coreOpacity = Clamp01(metrics.trailOpacity * 0.8);
    }
    return metrics;
}

ParticleSegmentMetrics ComputeParticleSegmentMetrics(
    double segmentRatio,
    double life,
    double intensity) {
    ParticleSegmentMetrics metrics{};
    const double safeRatio = Clamp01(segmentRatio);
    const double safeLife = Clamp01(life);
    const double safeIntensity = Clamp01(intensity);
    const double energy = ClampDouble(0.35 + safeIntensity * 0.65, 0.35, 1.0);

    metrics.radiusPx = std::max(
        0.6,
        (1.2 + 4.8 * safeRatio) * safeLife * (0.8 + 0.5 * energy));
    metrics.opacity = Clamp01(
        (0.28 + 0.72 * safeRatio) * safeLife * (0.55 + 0.45 * energy));
    metrics.emitHalo = (safeRatio > 0.55 && metrics.opacity > 0.03);
    if (metrics.emitHalo) {
        metrics.haloRadiusPx = metrics.radiusPx * (1.8 + 0.8 * energy);
        metrics.haloOpacity = Clamp01(metrics.opacity * 0.35);
    }
    return metrics;
}

} // namespace mousefx::trail_style_compute
