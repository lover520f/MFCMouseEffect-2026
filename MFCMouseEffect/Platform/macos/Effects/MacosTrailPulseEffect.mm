#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseEffect.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"

#include <chrono>
#include <cmath>
#include <utility>

namespace mousefx {
namespace {

TrailEffectThrottleProfile BuildComputeThrottleProfile(const macos_effect_profile::TrailThrottleProfile& profile) {
    TrailEffectThrottleProfile out{};
    out.minIntervalMs = profile.minIntervalMs;
    out.minDistancePx = profile.minDistancePx;
    return out;
}

TrailEffectProfile BuildComputeProfile(const macos_effect_profile::TrailRenderProfile& profile) {
    TrailEffectProfile out{};
    out.normalSizePx = profile.normalSizePx;
    out.particleSizePx = profile.particleSizePx;
    out.durationSec = profile.durationSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.line = {profile.line.fillArgb, profile.line.strokeArgb};
    out.streamer = {profile.streamer.fillArgb, profile.streamer.strokeArgb};
    out.electric = {profile.electric.fillArgb, profile.electric.strokeArgb};
    out.meteor = {profile.meteor.fillArgb, profile.meteor.strokeArgb};
    out.tubes = {profile.tubes.fillArgb, profile.tubes.strokeArgb};
    out.particle = {profile.particle.fillArgb, profile.particle.strokeArgb};
    out.lineTempo = {profile.lineTempo.durationScale, profile.lineTempo.sizeScale};
    out.streamerTempo = {profile.streamerTempo.durationScale, profile.streamerTempo.sizeScale};
    out.electricTempo = {profile.electricTempo.durationScale, profile.electricTempo.sizeScale};
    out.meteorTempo = {profile.meteorTempo.durationScale, profile.meteorTempo.sizeScale};
    out.tubesTempo = {profile.tubesTempo.durationScale, profile.tubesTempo.sizeScale};
    out.particleTempo = {profile.particleTempo.durationScale, profile.particleTempo.sizeScale};
    return out;
}

} // namespace

MacosTrailPulseEffect::MacosTrailPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::TrailRenderProfile renderProfile,
    macos_effect_profile::TrailThrottleProfile throttleProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile),
      throttleProfile_(throttleProfile) {
    if (effectType_.empty()) {
        effectType_ = "line";
    }
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
    const TrailEffectEmissionResult emission = ComputeTrailEffectEmission(
        pt,
        lastPoint_,
        now,
        lastEmitTickMs_,
        BuildComputeThrottleProfile(throttleProfile_));
    if (!emission.shouldEmit) {
        return;
    }

    lastEmitTickMs_ = now;
    lastPoint_ = pt;

    const TrailEffectRenderCommand command = ComputeTrailEffectRenderCommand(
        ScreenToOverlayPoint(pt),
        emission.deltaX,
        emission.deltaY,
        effectType_,
        BuildComputeProfile(renderProfile_));
    macos_trail_pulse::ShowTrailPulseOverlay(command, themeName_);
}

uint64_t MacosTrailPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx
