#include "pch.h"

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "Platform/macos/Effects/MacosHoverPulseEffect.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"

#include <utility>

namespace mousefx {
namespace {

HoverEffectProfile BuildComputeProfile(const macos_effect_profile::HoverRenderProfile& profile) {
    HoverEffectProfile out{};
    out.sizePx = profile.sizePx;
    out.breatheDurationSec = profile.breatheDurationSec;
    out.spinDurationSec = profile.spinDurationSec;
    out.baseOpacity = profile.baseOpacity;
    out.glowSizeScale = profile.glowSizeScale;
    out.tubesSizeScale = profile.tubesSizeScale;
    out.glowBreatheScale = profile.glowBreatheScale;
    out.tubesBreatheScale = profile.tubesBreatheScale;
    out.tubesSpinScale = profile.tubesSpinScale;
    out.colors = {profile.colors.glowFillArgb, profile.colors.glowStrokeArgb, profile.colors.tubesStrokeArgb};
    return out;
}

} // namespace

MacosHoverPulseEffect::MacosHoverPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::HoverRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile) {
    if (effectType_.empty()) {
        effectType_ = "glow";
    }
}

MacosHoverPulseEffect::~MacosHoverPulseEffect() {
    Shutdown();
}

bool MacosHoverPulseEffect::Initialize() {
    initialized_ = true;
    return true;
}

void MacosHoverPulseEffect::Shutdown() {
    initialized_ = false;
    macos_hover_pulse::CloseHoverPulseOverlay();
}

void MacosHoverPulseEffect::OnHoverStart(const ScreenPoint& pt) {
    if (!initialized_) {
        return;
    }
    const HoverEffectRenderCommand command =
        ComputeHoverEffectRenderCommand(ScreenToOverlayPoint(pt), effectType_, BuildComputeProfile(renderProfile_));
    macos_hover_pulse::ShowHoverPulseOverlay(command, themeName_);
}

void MacosHoverPulseEffect::OnHoverEnd() {
    macos_hover_pulse::CloseHoverPulseOverlay();
}

} // namespace mousefx
