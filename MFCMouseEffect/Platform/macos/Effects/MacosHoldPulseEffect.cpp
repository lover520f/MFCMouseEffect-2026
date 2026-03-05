#include "pch.h"

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "Platform/macos/Effects/MacosHoldPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"

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
      followMode_(ParseHoldEffectFollowMode(followMode)) {
    effectType_ = NormalizeHoldEffectType(effectType_);
}

MacosHoldPulseEffect::~MacosHoldPulseEffect() {
    Shutdown();
}

bool MacosHoldPulseEffect::Initialize() {
    initialized_ = true;
    running_ = false;
    followState_ = HoldEffectFollowState{};
    return true;
}

void MacosHoldPulseEffect::Shutdown() {
    initialized_ = false;
    running_ = false;
    followState_ = HoldEffectFollowState{};
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

    const HoldEffectStartCommand command = ComputeHoldEffectStartCommand(
        ScreenToOverlayPoint(pt),
        holdButton_,
        effectType_,
        macos_effect_compute_profile::BuildHoldProfile(renderProfile_));
    macos_hold_pulse::StartHoldPulseOverlay(command, themeName_);
    running_ = true;
    followState_ = HoldEffectFollowState{};
}

void MacosHoldPulseEffect::OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) {
    if (!initialized_ || !running_) {
        return;
    }
    const HoldEffectUpdateCommand command = ComputeHoldEffectUpdateCommand(
        ScreenToOverlayPoint(pt),
        durationMs,
        CurrentTickMs(),
        followMode_,
        &followState_);
    if (!command.emit) {
        return;
    }
    macos_hold_pulse::UpdateHoldPulseOverlay(command);
}

void MacosHoldPulseEffect::OnHoldEnd() {
    running_ = false;
    followState_ = HoldEffectFollowState{};
    macos_hold_pulse::StopHoldPulseOverlay();
}

bool MacosHoldPulseEffect::IsEffectActive() const {
    return running_;
}

uint64_t MacosHoldPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx
