#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseEffect.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"

#include <chrono>
#include <cmath>
#include <utility>

namespace mousefx {

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

    const double dx = static_cast<double>(pt.x - lastPoint_.x);
    const double dy = static_cast<double>(pt.y - lastPoint_.y);
    const double distance = std::sqrt(dx * dx + dy * dy);

    const uint64_t now = CurrentTickMs();
    if (distance < throttleProfile_.minDistancePx) {
        return;
    }
    if (lastEmitTickMs_ != 0 && now - lastEmitTickMs_ < throttleProfile_.minIntervalMs) {
        return;
    }

    lastEmitTickMs_ = now;
    lastPoint_ = pt;

    const ScreenPoint overlayPt = ScreenToOverlayPoint(pt);
    macos_trail_pulse::ShowTrailPulseOverlay(overlayPt, dx, dy, effectType_, themeName_, renderProfile_);
}

uint64_t MacosTrailPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx
