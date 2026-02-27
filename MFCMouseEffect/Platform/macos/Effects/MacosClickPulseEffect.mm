#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseEffect.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

#include <utility>

namespace mousefx {
namespace {

ClickEffectProfile BuildComputeProfile(const macos_effect_profile::ClickRenderProfile& profile) {
    ClickEffectProfile out{};
    out.normalSizePx = profile.normalSizePx;
    out.textSizePx = profile.textSizePx;
    out.normalDurationSec = profile.normalDurationSec;
    out.textDurationSec = profile.textDurationSec;
    out.closePaddingMs = profile.closePaddingMs;
    out.baseOpacity = profile.baseOpacity;
    out.left = {profile.leftButton.fillArgb, profile.leftButton.strokeArgb, profile.leftButton.glowArgb};
    out.right = {profile.rightButton.fillArgb, profile.rightButton.strokeArgb, profile.rightButton.glowArgb};
    out.middle = {profile.middleButton.fillArgb, profile.middleButton.strokeArgb, profile.middleButton.glowArgb};
    return out;
}

} // namespace

MacosClickPulseEffect::MacosClickPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::ClickRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile) {
    if (effectType_.empty()) {
        effectType_ = "ripple";
    }
}

MacosClickPulseEffect::~MacosClickPulseEffect() {
    Shutdown();
}

bool MacosClickPulseEffect::Initialize() {
    initialized_ = true;
    return true;
}

void MacosClickPulseEffect::Shutdown() {
    initialized_ = false;
    macos_click_pulse::CloseAllClickPulseWindows();
}

void MacosClickPulseEffect::OnClick(const ClickEvent& event) {
    if (!initialized_) {
        return;
    }
    const ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
        ScreenToOverlayPoint(event.pt),
        event.button,
        effectType_,
        BuildComputeProfile(renderProfile_));
    macos_click_pulse::ShowClickPulseOverlay(command, themeName_);
}

} // namespace mousefx
