#include "pch.h"

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "Platform/macos/Effects/MacosScrollPulseEffect.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"

#include <utility>

namespace mousefx {
namespace {

ScrollEffectProfile BuildComputeProfile(const macos_effect_profile::ScrollRenderProfile& profile) {
    ScrollEffectProfile out{};
    out.verticalSizePx = profile.verticalSizePx;
    out.horizontalSizePx = profile.horizontalSizePx;
    out.baseDurationSec = profile.baseDurationSec;
    out.perStrengthStepSec = profile.perStrengthStepSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.defaultDurationScale = profile.defaultDurationScale;
    out.helixDurationScale = profile.helixDurationScale;
    out.twinkleDurationScale = profile.twinkleDurationScale;
    out.defaultSizeScale = profile.defaultSizeScale;
    out.helixSizeScale = profile.helixSizeScale;
    out.twinkleSizeScale = profile.twinkleSizeScale;
    out.horizontalPositive = {profile.horizontalPositive.fillArgb, profile.horizontalPositive.strokeArgb};
    out.horizontalNegative = {profile.horizontalNegative.fillArgb, profile.horizontalNegative.strokeArgb};
    out.verticalPositive = {profile.verticalPositive.fillArgb, profile.verticalPositive.strokeArgb};
    out.verticalNegative = {profile.verticalNegative.fillArgb, profile.verticalNegative.strokeArgb};
    return out;
}

} // namespace

MacosScrollPulseEffect::MacosScrollPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::ScrollRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile) {
    if (effectType_.empty()) {
        effectType_ = "arrow";
    }
}

MacosScrollPulseEffect::~MacosScrollPulseEffect() {
    Shutdown();
}

bool MacosScrollPulseEffect::Initialize() {
    initialized_ = true;
    return true;
}

void MacosScrollPulseEffect::Shutdown() {
    initialized_ = false;
    macos_scroll_pulse::CloseAllScrollPulseWindows();
}

void MacosScrollPulseEffect::OnScroll(const ScrollEvent& event) {
    if (!initialized_ || event.delta == 0) {
        return;
    }

    const ScrollEffectRenderCommand command = ComputeScrollEffectRenderCommand(
        ScreenToOverlayPoint(event.pt),
        event.horizontal,
        event.delta,
        effectType_,
        BuildComputeProfile(renderProfile_));
    macos_scroll_pulse::ShowScrollPulseOverlay(command, themeName_);
}

} // namespace mousefx
