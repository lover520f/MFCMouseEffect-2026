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

double WrapHue(double hue) {
    double wrapped = std::fmod(hue, 360.0);
    if (wrapped < 0.0) {
        wrapped += 360.0;
    }
    return wrapped;
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
    const double forkLengthFactor = Clamp01((safeLength - 4.0) / 20.0);

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

    const double dynamicForkChance = safeFork * safeLife * (0.2 + 0.8 * forkLengthFactor);
    if (forkLengthFactor > 0.05 && Next01(&seed) < dynamicForkChance) {
        metrics.emitFork = true;
        metrics.forkT = NextRange(&seed, 0.35, 0.75);
        const double forkScale = ClampDouble(0.25 + 0.75 * forkLengthFactor, 0.25, 1.0);
        metrics.forkLengthPx = NextRange(&seed, 10.0, 22.0) * safeLife * forkScale;
        metrics.forkWidthPx = std::max(1.0, metrics.coreWidthPx * 1.2);
        // Keep electric fork visibility aligned with the Win renderer's glow alpha model.
        metrics.forkOpacity = metrics.glowOpacity;
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

int32_t ComputeParticleEmitCount(double distancePx) {
    if (distancePx < 1.0) {
        return 0;
    }
    int32_t emitCount = static_cast<int32_t>(distancePx * 0.18) + 2;
    emitCount = std::clamp(emitCount, int32_t{2}, int32_t{12});
    return emitCount;
}

ParticleSpawnMetrics ComputeParticleSpawnMetrics(
    uint32_t* state,
    bool chromatic,
    double globalHueDeg) {
    ParticleSpawnMetrics metrics{};
    constexpr double kTwoPi = 6.28318530717958647692;

    metrics.angleRad = NextRange(state, 0.0, kTwoPi);
    metrics.speedPxPerTick = NextRange(state, 0.5, 3.8);
    metrics.sizePx = NextRange(state, 2.0, 5.9);
    if (chromatic) {
        metrics.hueDeg = NextRange(state, 0.0, 359.999);
    } else {
        metrics.hueDeg = WrapHue(globalHueDeg + NextRange(state, -20.0, 20.0));
    }
    return metrics;
}

ParticleStepMetrics ComputeParticleStepMetrics(
    double x,
    double y,
    double vx,
    double vy,
    double life,
    double sizePx,
    double dtSec) {
    ParticleStepMetrics metrics{};
    const double safeDt = ClampDouble(dtSec, 0.0, 0.1);
    const double safeLife = Clamp01(life);
    const double safeSize = std::max(0.0, sizePx);

    metrics.nextX = x + vx;
    metrics.nextY = y + vy;
    metrics.nextVx = vx;
    metrics.nextVy = vy + 0.05;
    metrics.nextLife = safeLife - safeDt * 1.5;
    const double renderLife = Clamp01(metrics.nextLife);
    metrics.renderRadiusPx = std::max(0.0, safeSize * renderLife * 0.5);
    metrics.renderOpacity = renderLife;
    return metrics;
}

TubesNodeRenderMetrics ComputeTubesNodeRenderMetrics(
    uint32_t chainIndex,
    uint32_t nodeIndex,
    uint32_t nodesCount,
    double fadeScale) {
    TubesNodeRenderMetrics metrics{};
    const uint32_t safeCount = std::max<uint32_t>(1, nodesCount);
    const double invCount = 1.0 / static_cast<double>(safeCount);
    const double ratio = Clamp01(1.0 - static_cast<double>(nodeIndex) * invCount);
    const double safeFade = Clamp01(fadeScale);

    metrics.radiusPx = 2.0 + 7.0 * ratio;
    metrics.amplitudePx = 8.0;
    if (safeFade < 1.0) {
        metrics.radiusPx *= safeFade;
        metrics.amplitudePx *= safeFade;
    }
    metrics.alpha = Clamp01(ratio * safeFade);
    metrics.nodePhase = static_cast<double>(nodeIndex) * 0.3;
    metrics.chainPhase = static_cast<double>(chainIndex) * ((2.0 * 3.14159265358979323846) / 3.0);
    return metrics;
}

void ComputeTubesHeadFollow(
    double targetX,
    double targetY,
    double currentX,
    double currentY,
    double lag,
    double* outNextX,
    double* outNextY) {
    const double safeLag = ClampDouble(lag, 0.01, 1.0);
    const double dx = targetX - currentX;
    const double dy = targetY - currentY;
    if (outNextX != nullptr) {
        *outNextX = currentX + dx * safeLag;
    }
    if (outNextY != nullptr) {
        *outNextY = currentY + dy * safeLag;
    }
}

void ComputeTubesNodeFollow(
    double prevX,
    double prevY,
    double currentX,
    double currentY,
    double lag,
    double minSegmentDistance,
    double* outNextX,
    double* outNextY) {
    const double safeLag = ClampDouble(lag, 0.01, 1.0);
    const double safeMinDist = std::max(0.1, minSegmentDistance);
    const double ddx = prevX - currentX;
    const double ddy = prevY - currentY;
    double nextX = currentX + ddx * safeLag;
    double nextY = currentY + ddy * safeLag;

    const double dist = std::sqrt(ddx * ddx + ddy * ddy);
    if (dist < safeMinDist && dist > 0.01) {
        const double nx = ddx / dist;
        const double ny = ddy / dist;
        nextX = prevX - nx * safeMinDist;
        nextY = prevY - ny * safeMinDist;
    }

    if (outNextX != nullptr) {
        *outNextX = nextX;
    }
    if (outNextY != nullptr) {
        *outNextY = nextY;
    }
}

double ComputeTrailChromaticHueDeg(
    uint64_t nowMs,
    int32_t styleKind,
    uint32_t segmentIndex,
    uint32_t laneIndex) {
    const double t = static_cast<double>(nowMs);
    const double i = static_cast<double>(segmentIndex);
    const double lane = static_cast<double>(laneIndex);

    double hue = 0.0;
    switch (styleKind) {
    case 0: // line
        hue = t * 0.10 + i * 12.0;
        break;
    case 1: // streamer
        hue = t * 0.18 + i * 6.0;
        break;
    case 2: // electric
        hue = t * 0.55 + i * 18.0;
        break;
    case 3: // meteor
        hue = t * 0.15 + i * 8.0;
        break;
    case 4: // tubes
        hue = t * 0.20 + lane * 30.0;
        break;
    case 5: // particle
        hue = t * 0.20 + i * 10.0;
        break;
    default:
        hue = t * 0.12 + i * 10.0;
        break;
    }

    hue = std::fmod(hue, 360.0);
    if (hue < 0.0) {
        hue += 360.0;
    }
    return hue;
}

} // namespace mousefx::trail_style_compute
