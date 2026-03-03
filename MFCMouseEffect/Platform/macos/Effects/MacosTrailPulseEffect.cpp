#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosTrailPulseEmissionPlanner.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Effects/MacosLineTrailOverlay.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <utility>

namespace mousefx {
namespace {

std::atomic<uint64_t> gTrailMoveSamples{0};
std::atomic<uint64_t> gTrailOriginConnectorDropCount{0};
std::atomic<uint64_t> gTrailTeleportDropCount{0};

bool IsNearScreenOrigin(const ScreenPoint& pt) {
    constexpr int32_t kOriginTolerancePx = 6;
    return std::abs(pt.x) <= kOriginTolerancePx &&
           std::abs(pt.y) <= kOriginTolerancePx;
}

bool IsOriginConnectorSample(const ScreenPoint& from, const ScreenPoint& to) {
    const bool fromOrigin = IsNearScreenOrigin(from);
    const bool toOrigin = IsNearScreenOrigin(to);
    if (fromOrigin == toOrigin) {
        return false;
    }
    const double dx = static_cast<double>(to.x - from.x);
    const double dy = static_cast<double>(to.y - from.y);
    const double distance = std::sqrt(dx * dx + dy * dy);
    return distance >= 24.0;
}

bool IsContinuousTrailType(const std::string& normalizedType) {
    return normalizedType != "none";
}

macos_line_trail::LineTrailStyleKind ResolveLineTrailStyleKind(const std::string& normalizedType) {
    if (normalizedType == "streamer") {
        return macos_line_trail::LineTrailStyleKind::Streamer;
    }
    if (normalizedType == "electric") {
        return macos_line_trail::LineTrailStyleKind::Electric;
    }
    if (normalizedType == "meteor") {
        return macos_line_trail::LineTrailStyleKind::Meteor;
    }
    if (normalizedType == "tubes") {
        return macos_line_trail::LineTrailStyleKind::Tubes;
    }
    if (normalizedType == "particle") {
        return macos_line_trail::LineTrailStyleKind::Particle;
    }
    return macos_line_trail::LineTrailStyleKind::Line;
}

int ResolveTrailDurationFloorMs(const std::string& normalizedType) {
    if (normalizedType == "streamer") {
        return 420;
    }
    if (normalizedType == "electric") {
        return 280;
    }
    if (normalizedType == "meteor") {
        return 520;
    }
    if (normalizedType == "tubes") {
        return 350;
    }
    if (normalizedType == "particle") {
        return 240;
    }
    return 300;
}

float ResolveTrailLineWidthFloorPx(const std::string& normalizedType) {
    if (normalizedType == "streamer") {
        return 2.8f;
    }
    if (normalizedType == "electric") {
        return 2.2f;
    }
    if (normalizedType == "meteor") {
        return 2.6f;
    }
    if (normalizedType == "tubes") {
        return 3.0f;
    }
    if (normalizedType == "particle") {
        return 1.8f;
    }
    return 2.4f;
}

macos_line_trail::LineTrailConfig BuildLineTrailConfig(
    const TrailEffectRenderCommand& command,
    const std::string& themeName,
    const TrailRendererParamsConfig& trailParams,
    float fallbackLineWidth) {
    macos_line_trail::LineTrailConfig config{};
    const int computedDurationMs = static_cast<int>(std::lround(command.durationSec * 1000.0));
    const int durationFloorMs = ResolveTrailDurationFloorMs(command.normalizedType);
    config.durationMs = std::clamp(std::max(computedDurationMs, durationFloorMs), 80, 2200);

    const float fallbackWidth = std::clamp(fallbackLineWidth, 1.0f, 24.0f);
    float lineWidth = (command.lineWidthPx > 0.0)
        ? static_cast<float>(command.lineWidthPx)
        : fallbackWidth;
    if (command.normalizedType == "streamer") {
        lineWidth *= 1.18f;
    } else if (command.normalizedType == "tubes") {
        lineWidth *= 1.24f;
    }
    config.lineWidth = std::clamp(
        std::max(lineWidth, ResolveTrailLineWidthFloorPx(command.normalizedType)),
        1.0f,
        24.0f);
    config.strokeArgb = command.strokeArgb;
    config.fillArgb = command.fillArgb;
    if ((config.fillArgb & 0xFF000000u) == 0u) {
        config.fillArgb = (0x66u << 24) | (config.strokeArgb & 0x00FFFFFFu);
    }
    config.intensity = std::clamp(command.intensity, 0.0, 1.0);
    config.style = ResolveLineTrailStyleKind(command.normalizedType);
    config.chromatic = (ToLowerAscii(themeName) == "chromatic");
    config.streamerGlowWidthScale = trailParams.streamer.glowWidthScale;
    config.streamerCoreWidthScale = trailParams.streamer.coreWidthScale;
    config.streamerHeadPower = trailParams.streamer.headPower;
    config.electricAmplitudeScale = trailParams.electric.amplitudeScale;
    config.electricForkChance = trailParams.electric.forkChance;
    config.meteorSparkRateScale = trailParams.meteor.sparkRateScale;
    config.meteorSparkSpeedScale = trailParams.meteor.sparkSpeedScale;
    config.idleFade.startMs = std::max(0, trailParams.idleFade.startMs);
    config.idleFade.endMs = std::max(config.idleFade.startMs + 1, trailParams.idleFade.endMs);
    return config;
}

double ResolveContinuousTrailStepPx(const std::string& normalizedType) {
    if (normalizedType == "streamer") {
        return 3.0;
    }
    if (normalizedType == "electric") {
        return 2.5;
    }
    if (normalizedType == "meteor") {
        return 3.0;
    }
    if (normalizedType == "tubes") {
        return 3.5;
    }
    if (normalizedType == "particle") {
        return 2.2;
    }
    return 4.0;
}

} // namespace

TrailPulseRuntimeDiagnostics ReadTrailPulseRuntimeDiagnostics() {
    TrailPulseRuntimeDiagnostics diag{};
    diag.moveSamples = gTrailMoveSamples.load(std::memory_order_relaxed);
    diag.originConnectorDropCount = gTrailOriginConnectorDropCount.load(std::memory_order_relaxed);
    diag.teleportDropCount = gTrailTeleportDropCount.load(std::memory_order_relaxed);
    return diag;
}

MacosTrailPulseEffect::MacosTrailPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::TrailRenderProfile renderProfile,
    macos_effect_profile::TrailThrottleProfile throttleProfile,
    TrailRendererParamsConfig trailParams,
    float lineWidth)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile),
      throttleProfile_(throttleProfile),
      trailParams_(trailParams),
      lineWidth_(lineWidth) {
    effectType_ = NormalizeTrailEffectType(effectType_);
}

MacosTrailPulseEffect::~MacosTrailPulseEffect() {
    Shutdown();
}

bool MacosTrailPulseEffect::Initialize() {
    initialized_ = true;
    hasLastPoint_ = false;
    lastEmitTickMs_ = 0;
    continuousTrailActive_ = false;
    emissionPlannerConfig_ = macos_trail_pulse::ResolveTrailPulseEmissionPlannerConfig();
    return true;
}

void MacosTrailPulseEffect::Shutdown() {
    initialized_ = false;
    hasLastPoint_ = false;
    lastEmitTickMs_ = 0;
    continuousTrailActive_ = false;
    macos_line_trail::ResetLineTrail();
    macos_trail_pulse::CloseAllTrailPulseWindows();
}

void MacosTrailPulseEffect::OnMouseMove(const ScreenPoint& pt) {
    if (!initialized_) {
        return;
    }

    if (!hasLastPoint_) {
        hasLastPoint_ = true;
        lastPoint_ = pt;
        return;
    }
    gTrailMoveSamples.fetch_add(1, std::memory_order_relaxed);

    const uint64_t now = CurrentTickMs();
    const std::string normalizedType = NormalizeTrailEffectType(effectType_);
    if (normalizedType == "none") {
        if (continuousTrailActive_) {
            macos_line_trail::ResetLineTrail();
            continuousTrailActive_ = false;
        }
        lastPoint_ = pt;
        return;
    }
    const bool continuousTrail = IsContinuousTrailType(normalizedType);
    if (!continuousTrail && continuousTrailActive_) {
        macos_line_trail::ResetLineTrail();
        continuousTrailActive_ = false;
    }
    const double moveDx = static_cast<double>(pt.x - lastPoint_.x);
    const double moveDy = static_cast<double>(pt.y - lastPoint_.y);
    const double moveDistance = std::sqrt(moveDx * moveDx + moveDy * moveDy);
    if (IsOriginConnectorSample(lastPoint_, pt)) {
        gTrailOriginConnectorDropCount.fetch_add(1, std::memory_order_relaxed);
        if (continuousTrail && continuousTrailActive_) {
            macos_line_trail::ResetLineTrail();
            continuousTrailActive_ = false;
        }
        lastPoint_ = pt;
        return;
    }
    if (moveDistance > std::max(200.0, emissionPlannerConfig_.teleportSkipDistancePx)) {
        gTrailTeleportDropCount.fetch_add(1, std::memory_order_relaxed);
        if (continuousTrail && continuousTrailActive_) {
            macos_line_trail::ResetLineTrail();
            continuousTrailActive_ = false;
        }
        lastPoint_ = pt;
        return;
    }

    if (continuousTrail) {
        const TrailEffectProfile profile =
            macos_effect_compute_profile::BuildTrailProfile(renderProfile_);
        const double segmentStepPx = ResolveContinuousTrailStepPx(normalizedType);
        const int segmentCount = static_cast<int>(std::clamp(std::ceil(moveDistance / segmentStepPx), 1.0, 72.0));
        ScreenPoint prev = lastPoint_;
        for (int i = 1; i <= segmentCount; ++i) {
            const double t = static_cast<double>(i) / static_cast<double>(segmentCount);
            ScreenPoint segPt{};
            segPt.x = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.x) +
                                                       (static_cast<double>(pt.x - lastPoint_.x) * t)));
            segPt.y = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.y) +
                                                       (static_cast<double>(pt.y - lastPoint_.y) * t)));
            const TrailEffectRenderCommand command = ComputeTrailEffectRenderCommand(
                ScreenToOverlayPoint(segPt),
                static_cast<double>(segPt.x - prev.x),
                static_cast<double>(segPt.y - prev.y),
                normalizedType,
                profile);
            if (command.emit) {
                const macos_line_trail::LineTrailConfig config =
                    BuildLineTrailConfig(command, themeName_, trailParams_, lineWidth_);
                macos_line_trail::UpdateLineTrail(segPt, config);
            }
            prev = segPt;
        }
        continuousTrailActive_ = true;
        lastPoint_ = pt;
        return;
    }
    const auto throttleProfile =
        macos_effect_compute_profile::BuildTrailThrottleProfile(throttleProfile_);
    TrailEffectEmissionResult emission = ComputeTrailEffectEmission(
        pt,
        lastPoint_,
        now,
        lastEmitTickMs_,
        throttleProfile);
    if (!emission.shouldEmit) {
        const double forceDistance = std::max(12.0, throttleProfile.minDistancePx * 2.0);
        if (emission.distancePx < forceDistance) {
            return;
        }
        emission.shouldEmit = true;
    }

    lastEmitTickMs_ = now;
    const TrailEffectProfile profile =
        macos_effect_compute_profile::BuildTrailProfile(renderProfile_);
    const macos_trail_pulse::TrailPulseEmissionPlan segmentPlan =
        macos_trail_pulse::BuildTrailPulseEmissionPlan(
            lastPoint_,
            pt,
            normalizedType,
            throttleProfile.minDistancePx,
            emissionPlannerConfig_);
    if (segmentPlan.dropAsTeleport || segmentPlan.segmentPoints.empty()) {
        if (segmentPlan.dropAsTeleport) {
            gTrailTeleportDropCount.fetch_add(1, std::memory_order_relaxed);
        }
        lastPoint_ = pt;
        return;
    }

    ScreenPoint prev = lastPoint_;
    for (const ScreenPoint& segPt : segmentPlan.segmentPoints) {
        const double dx = static_cast<double>(segPt.x - prev.x);
        const double dy = static_cast<double>(segPt.y - prev.y);
        const TrailEffectRenderCommand command = ComputeTrailEffectRenderCommand(
            ScreenToOverlayPoint(segPt),
            dx,
            dy,
            normalizedType,
            profile);
        macos_trail_pulse::ShowTrailPulseOverlay(command, themeName_);
        prev = segPt;
    }
    lastPoint_ = pt;
}

uint64_t MacosTrailPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx
