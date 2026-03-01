#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosTrailPulseEmissionPlanner.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosLineTrailOverlay.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

namespace mousefx {

MacosTrailPulseEffect::MacosTrailPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::TrailRenderProfile renderProfile,
    macos_effect_profile::TrailThrottleProfile throttleProfile,
    IdleFadeParams idleFade,
    float lineWidth)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile),
      throttleProfile_(throttleProfile),
      idleFade_(idleFade),
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
    const bool lineTrail = (normalizedType == "line");
    const bool streamerTrail = (normalizedType == "streamer");
    const bool continuousTrail = (lineTrail || streamerTrail);
    if (!continuousTrail && continuousTrailActive_) {
        macos_line_trail::ResetLineTrail();
        continuousTrailActive_ = false;
    }
    const double moveDx = static_cast<double>(pt.x - lastPoint_.x);
    const double moveDy = static_cast<double>(pt.y - lastPoint_.y);
    const double moveDistance = std::sqrt(moveDx * moveDx + moveDy * moveDy);
    if (moveDistance > std::max(200.0, emissionPlannerConfig_.teleportSkipDistancePx)) {
        if (continuousTrail && continuousTrailActive_) {
            macos_line_trail::ResetLineTrail();
            continuousTrailActive_ = false;
        }
        lastPoint_ = pt;
        return;
    }

    if (continuousTrail) {
        macos_line_trail::LineTrailConfig config{};
        // Keep line/streamer on continuous overlay path to avoid pulse-segment
        // "matchstick" artifacts during fast pointer movement.
        const double durationScale = streamerTrail ? 1.9 : 2.5;
        const int minDurationMs = streamerTrail ? 320 : 500;
        const int maxDurationMs = streamerTrail ? 1800 : 2200;
        config.durationMs = std::clamp(
            static_cast<int>(std::lround(renderProfile_.durationSec * 1000.0 * durationScale)),
            minDurationMs,
            maxDurationMs);
        config.lineWidth = std::clamp(
            lineWidth_ * (streamerTrail ? 0.9f : 1.0f),
            streamerTrail ? 1.5f : 2.0f,
            18.0f);
        config.strokeArgb = streamerTrail ? renderProfile_.streamer.strokeArgb : renderProfile_.line.strokeArgb;
        config.idleFade.startMs = std::max(idleFade_.startMs, streamerTrail ? 180 : 300);
        config.idleFade.endMs = std::max(idleFade_.endMs, streamerTrail ? 420 : 600);
        const double distance = moveDistance;
        const double segmentStepPx = streamerTrail ? 3.0 : 4.0;
        const int segmentCount = static_cast<int>(std::clamp(std::ceil(distance / segmentStepPx), 1.0, 64.0));
        for (int i = 1; i <= segmentCount; ++i) {
            const double t = static_cast<double>(i) / static_cast<double>(segmentCount);
            ScreenPoint segPt{};
            segPt.x = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.x) +
                                                       (static_cast<double>(pt.x - lastPoint_.x) * t)));
            segPt.y = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.y) +
                                                       (static_cast<double>(pt.y - lastPoint_.y) * t)));
            macos_line_trail::UpdateLineTrail(segPt, config);
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
        const double forceDistance = (normalizedType == "streamer")
            ? std::max(6.0, throttleProfile.minDistancePx * 1.4)
            : std::max(12.0, throttleProfile.minDistancePx * 2.0);
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
