#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"

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
    return true;
}

void MacosTrailPulseEffect::Shutdown() {
    initialized_ = false;
    hasLastPoint_ = false;
    lastEmitTickMs_ = 0;
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
        return;
    }
    const bool lineTrail = (normalizedType == "line");

    if (lineTrail) {
        macos_line_trail::LineTrailConfig config{};
        // Use a longer duration for continuous line trail than the pulse trail.
        // The pulse durationSec is scaled down by 0.73 which is too short for
        // a persistent trail. Use at least 500ms for good visibility.
        config.durationMs = std::clamp(
            static_cast<int>(std::lround(renderProfile_.durationSec * 1000.0 * 2.5)),
            500,
            2000);
        config.lineWidth = std::clamp(lineWidth_, 2.0f, 18.0f);
        config.strokeArgb = renderProfile_.line.strokeArgb;
        // Use gentler idle fade for continuous line trail
        config.idleFade.startMs = std::max(idleFade_.startMs, 300);
        config.idleFade.endMs = std::max(idleFade_.endMs, 600);
        const double dx = static_cast<double>(pt.x - lastPoint_.x);
        const double dy = static_cast<double>(pt.y - lastPoint_.y);
        const double distance = std::sqrt(dx * dx + dy * dy);
        const int segmentCount = static_cast<int>(std::clamp(std::ceil(distance / 4.0), 1.0, 48.0));
        for (int i = 1; i <= segmentCount; ++i) {
            const double t = static_cast<double>(i) / static_cast<double>(segmentCount);
            ScreenPoint segPt{};
            segPt.x = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.x) +
                                                       (static_cast<double>(pt.x - lastPoint_.x) * t)));
            segPt.y = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.y) +
                                                       (static_cast<double>(pt.y - lastPoint_.y) * t)));
            macos_line_trail::UpdateLineTrail(segPt, config);
        }
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
    const double distance = std::max(0.0, emission.distancePx);
    const double minStep = (normalizedType == "streamer")
        ? 3.5
        : ((normalizedType == "meteor") ? 6.0 : ((normalizedType == "electric") ? 5.0 : 8.0));
    const double segmentStep = std::max(minStep, throttleProfile.minDistancePx * 0.9);
    const int segmentCount = static_cast<int>(std::clamp(std::ceil(distance / segmentStep), 1.0, 48.0));
    ScreenPoint prev = lastPoint_;
    for (int i = 1; i <= segmentCount; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(segmentCount);
        ScreenPoint segPt{};
        segPt.x = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.x) +
                                                   (static_cast<double>(pt.x - lastPoint_.x) * t)));
        segPt.y = static_cast<int32_t>(std::lround(static_cast<double>(lastPoint_.y) +
                                                   (static_cast<double>(pt.y - lastPoint_.y) * t)));
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
