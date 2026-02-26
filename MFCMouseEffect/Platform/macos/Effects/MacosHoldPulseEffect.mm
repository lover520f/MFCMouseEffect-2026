#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseEffect.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"

#include <chrono>
#include <cmath>
#include <utility>

namespace mousefx {

MacosHoldPulseEffect::MacosHoldPulseEffect(
    std::string effectType,
    std::string themeName,
    std::string followMode,
    macos_effect_profile::HoldRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile),
      followMode_(ParseFollowMode(followMode)) {
    if (effectType_.empty()) {
        effectType_ = "charge";
    }
}

MacosHoldPulseEffect::~MacosHoldPulseEffect() {
    Shutdown();
}

bool MacosHoldPulseEffect::Initialize() {
    initialized_ = true;
    running_ = false;
    hasSmoothedPoint_ = false;
    lastEfficientTickMs_ = 0;
    return true;
}

void MacosHoldPulseEffect::Shutdown() {
    initialized_ = false;
    running_ = false;
    hasSmoothedPoint_ = false;
    lastEfficientTickMs_ = 0;
    macos_hold_pulse::StopHoldPulseOverlay();
}

void MacosHoldPulseEffect::OnHoldStart(const ScreenPoint& pt, int button) {
    if (!initialized_) {
        return;
    }

    if (button == static_cast<int>(MouseButton::Right)) {
        holdButton_ = MouseButton::Right;
    } else if (button == static_cast<int>(MouseButton::Middle)) {
        holdButton_ = MouseButton::Middle;
    } else {
        holdButton_ = MouseButton::Left;
    }

    const ScreenPoint overlayPt = ScreenToOverlayPoint(pt);
    macos_hold_pulse::StartHoldPulseOverlay(overlayPt, holdButton_, effectType_, themeName_, renderProfile_);
    running_ = true;
    hasSmoothedPoint_ = false;
    lastEfficientTickMs_ = 0;
}

void MacosHoldPulseEffect::OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) {
    if (!initialized_ || !running_) {
        return;
    }

    ScreenPoint updatePoint = pt;
    const uint64_t nowMs = CurrentTickMs();

    switch (followMode_) {
    case FollowMode::Precise:
        break;
    case FollowMode::Smooth:
        if (!hasSmoothedPoint_) {
            smoothedX_ = static_cast<float>(pt.x);
            smoothedY_ = static_cast<float>(pt.y);
            hasSmoothedPoint_ = true;
        } else {
            constexpr float kAlpha = 0.35f;
            smoothedX_ += (static_cast<float>(pt.x) - smoothedX_) * kAlpha;
            smoothedY_ += (static_cast<float>(pt.y) - smoothedY_) * kAlpha;
        }
        updatePoint.x = static_cast<int32_t>(std::lround(smoothedX_));
        updatePoint.y = static_cast<int32_t>(std::lround(smoothedY_));
        break;
    case FollowMode::Efficient:
        if (lastEfficientTickMs_ != 0 && (nowMs - lastEfficientTickMs_) < 20) {
            return;
        }
        lastEfficientTickMs_ = nowMs;
        break;
    }

    macos_hold_pulse::UpdateHoldPulseOverlay(ScreenToOverlayPoint(updatePoint), durationMs, renderProfile_);
}

void MacosHoldPulseEffect::OnHoldEnd() {
    running_ = false;
    hasSmoothedPoint_ = false;
    lastEfficientTickMs_ = 0;
    macos_hold_pulse::StopHoldPulseOverlay();
}

MacosHoldPulseEffect::FollowMode MacosHoldPulseEffect::ParseFollowMode(const std::string& mode) {
    const std::string normalized = ToLowerAscii(mode);
    if (normalized == "precise") {
        return FollowMode::Precise;
    }
    if (normalized == "efficient") {
        return FollowMode::Efficient;
    }
    return FollowMode::Smooth;
}

uint64_t MacosHoldPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx
